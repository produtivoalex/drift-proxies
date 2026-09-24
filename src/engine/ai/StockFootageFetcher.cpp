// src/engine/ai/StockFootageFetcher.cpp
// Fase 5B — Auto-Fetch B-Rolls (implementação)

#include "StockFootageFetcher.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QSettings>
#include <QStandardPaths>
#include <QUrl>
#include <QUrlQuery>

namespace drift {

// ── Cache directory ─────────────────────────────────────────────────────────

QString StockFootageFetcher::cacheDir()
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QString dir  = base + QStringLiteral("/broll_cache");
    QDir().mkpath(dir);
    return dir;
}

void StockFootageFetcher::pruneCache(int olderThanDays)
{
    const QDir dir(cacheDir());
    const QDateTime cutoff = QDateTime::currentDateTimeUtc().addDays(-olderThanDays);
    const QFileInfoList files = dir.entryInfoList(QDir::Files);
    for (const QFileInfo &fi : files) {
        if (fi.lastModified().toUTC() < cutoff)
            QFile::remove(fi.absoluteFilePath());
    }
}

QString StockFootageFetcher::cachePathFor(const QString &url) const
{
    const QByteArray hash = QCryptographicHash::hash(url.toUtf8(), QCryptographicHash::Md5).toHex();
    return cacheDir() + QStringLiteral("/") + QString::fromLatin1(hash) + QStringLiteral(".mp4");
}

// ── Constructor / destructor ────────────────────────────────────────────────

StockFootageFetcher::StockFootageFetcher(QObject *parent)
    : QObject(parent)
{
    QSettings s;
    m_pexelsKey  = s.value(QStringLiteral("ai/pexelsApiKey")).toString();
    m_pixabayKey = s.value(QStringLiteral("ai/pixabayApiKey")).toString();
}

StockFootageFetcher::~StockFootageFetcher()
{
    cancel();
}

// ── Configuration ───────────────────────────────────────────────────────────

void StockFootageFetcher::setPexelsApiKey(const QString &key)
{
    m_pexelsKey = key;
    QSettings().setValue(QStringLiteral("ai/pexelsApiKey"), key);
}

void StockFootageFetcher::setPixabayApiKey(const QString &key)
{
    m_pixabayKey = key;
    QSettings().setValue(QStringLiteral("ai/pixabayApiKey"), key);
}

void StockFootageFetcher::cancel()
{
    m_cancelled = true;
}

// ── Main entry point ────────────────────────────────────────────────────────

void StockFootageFetcher::fetchBRolls(const QStringList &queries,
                                       int maxPerQuery,
                                       int minDurationSec,
                                       int maxDurationSec,
                                       bool portrait)
{
    if (queries.isEmpty() || !hasAnyKey()) {
        emit fetchFinished(0, 0);
        return;
    }

    m_cancelled = false;
    m_successCount.storeRelaxed(0);
    m_failCount.storeRelaxed(0);
    m_totalQueries.storeRelaxed(queries.size());
    m_pendingQueries.storeRelaxed(queries.size());

    emit progressChanged(0.02, tr("Buscando B-Rolls para %1 cenas...").arg(queries.size()));

    // Prefer Pexels (richer video library), fall back to Pixabay
    for (const QString &q : queries) {
        if (m_cancelled) break;
        if (hasPexelsKey())
            searchPexels(q, maxPerQuery, minDurationSec, maxDurationSec, portrait);
        else
            searchPixabay(q, maxPerQuery, minDurationSec, maxDurationSec, portrait);
    }
}

// ── Pexels API ──────────────────────────────────────────────────────────────

void StockFootageFetcher::searchPexels(const QString &query, int maxResults,
                                        int minSec, int maxSec, bool portrait)
{
    // Pexels Videos Search: GET https://api.pexels.com/videos/search
    QUrlQuery params;
    params.addQueryItem(QStringLiteral("query"),       query);
    params.addQueryItem(QStringLiteral("per_page"),    QString::number(qMin(maxResults * 4, 15))); // fetch more, filter locally
    params.addQueryItem(QStringLiteral("size"),        QStringLiteral("medium"));
    if (portrait)
        params.addQueryItem(QStringLiteral("orientation"), QStringLiteral("portrait"));
    else
        params.addQueryItem(QStringLiteral("orientation"), QStringLiteral("landscape"));

    QUrl url(QStringLiteral("https://api.pexels.com/videos/search"));
    url.setQuery(params);

    QNetworkRequest req(url);
    req.setRawHeader("Authorization", m_pexelsKey.toUtf8());

    QNetworkReply *reply = m_net.get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply, query, maxResults, minSec, maxSec]() {
        reply->deleteLater();

        if (m_cancelled) { onQueryDone(); return; }

        if (reply->error() != QNetworkReply::NoError) {
            // Try Pixabay as fallback if we have a key
            if (hasPixabayKey())
                searchPixabay(query, maxResults, minSec, maxSec, false);
            else {
                m_failCount.fetchAndAddRelaxed(1);
                onQueryDone();
            }
            return;
        }

        const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        const QJsonArray videos = doc[QStringLiteral("videos")].toArray();

        int found = 0;
        for (const QJsonValue &v : videos) {
            if (found >= maxResults || m_cancelled) break;
            const QJsonObject video = v.toObject();
            const int duration = video[QStringLiteral("duration")].toInt();
            if (duration < minSec || duration > maxSec) continue;

            // Pick the best video file: prefer HD (1280x720), avoid 4K to save bandwidth
            const QJsonArray files = video[QStringLiteral("video_files")].toArray();
            QString bestUrl;
            int bestWidth = 0;
            for (const QJsonValue &f : files) {
                const QJsonObject file = f.toObject();
                const int w = file[QStringLiteral("width")].toInt();
                // Target 720p-1080p: skip >1920 and <640
                if (w >= 640 && w <= 1920 && w > bestWidth) {
                    bestWidth = w;
                    bestUrl = file[QStringLiteral("link")].toString();
                }
            }
            if (bestUrl.isEmpty()) continue;

            const QString previewUrl = video[QStringLiteral("image")].toString();
            const int height = video[QStringLiteral("height")].toInt();
            downloadVideo(query, bestUrl, previewUrl, duration, bestWidth, height, QStringLiteral("pexels"));
            ++found;
        }

        if (found == 0) {
            // No valid result — try Pixabay fallback
            if (hasPixabayKey())
                searchPixabay(query, maxResults, minSec, maxSec, false);
            else {
                m_failCount.fetchAndAddRelaxed(1);
                onQueryDone();
            }
        }
    });
}

// ── Pixabay API ─────────────────────────────────────────────────────────────

void StockFootageFetcher::searchPixabay(const QString &query, int maxResults,
                                         int minSec, int maxSec, bool portrait)
{
    Q_UNUSED(portrait); // Pixabay doesn't filter by orientation for video

    // Pixabay Video API: GET https://pixabay.com/api/videos/
    QUrlQuery params;
    params.addQueryItem(QStringLiteral("key"),         m_pixabayKey);
    params.addQueryItem(QStringLiteral("q"),           QUrl::toPercentEncoding(query));
    params.addQueryItem(QStringLiteral("video_type"), QStringLiteral("film"));
    params.addQueryItem(QStringLiteral("per_page"),    QString::number(qMin(maxResults * 3, 15)));
    params.addQueryItem(QStringLiteral("safesearch"),  QStringLiteral("true"));

    QUrl url(QStringLiteral("https://pixabay.com/api/videos/"));
    url.setQuery(params);

    QNetworkRequest req(url);
    QNetworkReply *reply = m_net.get(req);

    connect(reply, &QNetworkReply::finished, this, [this, reply, query, maxResults, minSec, maxSec]() {
        reply->deleteLater();

        if (m_cancelled) { onQueryDone(); return; }

        if (reply->error() != QNetworkReply::NoError) {
            m_failCount.fetchAndAddRelaxed(1);
            onQueryDone();
            return;
        }

        const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        const QJsonArray hits = doc[QStringLiteral("hits")].toArray();

        int found = 0;
        for (const QJsonValue &v : hits) {
            if (found >= maxResults || m_cancelled) break;
            const QJsonObject hit    = v.toObject();
            const int duration       = hit[QStringLiteral("duration")].toInt();
            if (duration < minSec || duration > maxSec) continue;

            const QJsonObject videos = hit[QStringLiteral("videos")].toObject();

            // Prefer medium (1280x720), fall back to small (960px), then tiny
            QString bestUrl;
            int bestWidth = 0;
            int bestHeight = 0;
            const QStringList sizes = {QStringLiteral("medium"), QStringLiteral("small"), QStringLiteral("tiny")};
            for (const QString &sz : sizes) {
                if (!videos.contains(sz)) continue;
                const QJsonObject vobj = videos[sz].toObject();
                const int w = vobj[QStringLiteral("width")].toInt();
                if (w > bestWidth) {
                    bestWidth  = w;
                    bestHeight = vobj[QStringLiteral("height")].toInt();
                    bestUrl    = vobj[QStringLiteral("url")].toString();
                }
            }
            if (bestUrl.isEmpty()) continue;

            const QString preview = hit[QStringLiteral("picture_id")].toString().isEmpty()
                ? QString()
                : QStringLiteral("https://i.vimeocdn.com/video/%1_295x166.jpg")
                      .arg(hit[QStringLiteral("picture_id")].toString());

            downloadVideo(query, bestUrl, preview, duration, bestWidth, bestHeight, QStringLiteral("pixabay"));
            ++found;
        }

        if (found == 0)
            m_failCount.fetchAndAddRelaxed(1);

        onQueryDone();
    });
}

// ── Video download ──────────────────────────────────────────────────────────

void StockFootageFetcher::downloadVideo(const QString &query, const QString &videoUrl,
                                         const QString &previewUrl, int durationSec,
                                         int width, int height, const QString &source)
{
    const QString dest = cachePathFor(videoUrl);

    // Cache hit — no need to download again
    if (QFile::exists(dest)) {
        BRollResult r;
        r.query        = query;
        r.localPath    = dest;
        r.previewUrl   = previewUrl;
        r.videoUrl     = videoUrl;
        r.durationSec  = durationSec;
        r.width        = width;
        r.height       = height;
        r.source       = source;
        r.success      = true;
        m_successCount.fetchAndAddRelaxed(1);
        emit brollReady(r);
        return;
    }

    // Download to temp, then rename atomically
    const QString tmpDest = dest + QStringLiteral(".tmp");
    auto *file = new QFile(tmpDest);
    if (!file->open(QIODevice::WriteOnly)) {
        delete file;
        m_failCount.fetchAndAddRelaxed(1);
        return;
    }

    QNetworkRequest req((QUrl(videoUrl)));
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);
    QNetworkReply *reply = m_net.get(req);

    connect(reply, &QNetworkReply::readyRead, this, [reply, file]() {
        file->write(reply->readAll());
    });

    connect(reply, &QNetworkReply::finished, this,
            [this, reply, file, dest, tmpDest, query, videoUrl, previewUrl,
             durationSec, width, height, source]() {
        reply->deleteLater();
        file->close();
        file->deleteLater();

        if (m_cancelled || reply->error() != QNetworkReply::NoError) {
            QFile::remove(tmpDest);
            m_failCount.fetchAndAddRelaxed(1);
            return;
        }

        QFile::rename(tmpDest, dest);

        BRollResult r;
        r.query        = query;
        r.localPath    = dest;
        r.previewUrl   = previewUrl;
        r.videoUrl     = videoUrl;
        r.durationSec  = durationSec;
        r.width        = width;
        r.height       = height;
        r.source       = source;
        r.success      = true;
        m_successCount.fetchAndAddRelaxed(1);
        emit brollReady(r);
    });
}

// ── Completion tracking ─────────────────────────────────────────────────────

void StockFootageFetcher::onQueryDone()
{
    const int remaining = m_pendingQueries.fetchAndSubRelaxed(1) - 1;
    const int total     = m_totalQueries.loadRelaxed();
    const double done   = total > 0 ? double(total - remaining) / double(total) : 1.0;

    emit progressChanged(done, tr("%1/%2 B-Rolls prontos").arg(total - remaining).arg(total));

    if (remaining <= 0) {
        emit fetchFinished(m_successCount.loadRelaxed(), m_failCount.loadRelaxed());
    }
}

} // namespace drift
