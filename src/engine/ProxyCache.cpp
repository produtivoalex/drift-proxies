#include "ProxyCache.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QUuid>

#include <algorithm>
#include <vector>

namespace drift {

namespace {

QString indexPath()
{
    const QString dir = ProxyCache::proxyCacheDir();
    return dir.isEmpty() ? QString() : QDir(dir).filePath(QStringLiteral("index.json"));
}

} // namespace

ProxyCache &ProxyCache::instance()
{
    static ProxyCache cache;
    return cache;
}

QString ProxyCache::proxyCacheDir()
{
    static const QString dir = [] {
        const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        const QString path = QDir(base).filePath(QStringLiteral("proxies"));
        QDir().mkpath(path);
        return path;
    }();
    return dir;
}

QString ProxyCache::newProxyPath(const QString &sourcePath)
{
    const QString dir = proxyCacheDir();
    if (dir.isEmpty())
        return {};

    const QFileInfo src(sourcePath);
    const QString cleanName = src.baseName().left(20).replace(QLatin1Char(' '), QLatin1Char('_'));
    const QString uuid = QUuid::createUuid().toString(QUuid::Id128).left(10);
    const QString fileName = QStringLiteral("proxy_%1_%2.mp4").arg(cleanName, uuid);
    return QDir(dir).filePath(fileName);
}

bool ProxyCache::isEligible(int width, int height)
{
    if (width <= 0 || height <= 0)
        return false;

    // Any video 720p or higher (horizontal: 1280x720, vertical: 720x1280, or total pixels >= 1280*720)
    return (width >= 1280 || height >= 720 || (width * height >= 1280 * 720));
}

QSize ProxyCache::targetProxySize(int srcWidth, int srcHeight)
{
    if (srcWidth <= 0 || srcHeight <= 0)
        return {960, 540};

    const int shortEdge = qMin(srcWidth, srcHeight);
    const double aspect = static_cast<double>(srcWidth) / srcHeight;

    // For 4K and above, generate a 720p proxy. For 1080p and 720p, generate a 540p proxy.
    const int targetShort = (shortEdge >= 1440) ? 720 : 540;

    int targetW = 0;
    int targetH = 0;

    if (srcWidth >= srcHeight) {
        // Landscape
        targetH = targetShort;
        targetW = static_cast<int>(std::round(targetH * aspect));
    } else {
        // Portrait
        targetW = targetShort;
        targetH = static_cast<int>(std::round(targetW / aspect));
    }

    // Must be even dimensions for YUV420P / H.264
    targetW = qMax(2, targetW & ~1);
    targetH = qMax(2, targetH & ~1);

    return {targetW, targetH};
}

QString ProxyCache::lookup(const QString &sourcePath) const
{
    if (sourcePath.isEmpty())
        return {};

    const QFileInfo info(sourcePath);
    const QString key = info.absoluteFilePath();

    QMutexLocker lock(&m_mutex);
    auto it = m_entries.find(key);
    if (it == m_entries.end())
        return {};

    const qint64 mtimeMs = info.lastModified().toMSecsSinceEpoch();
    const qint64 size = info.size();

    // Verify source has not changed and proxy file physically exists
    if (it->sourceMtimeMs != mtimeMs || it->sourceSize != size || !QFile::exists(it->proxyPath))
        return {};

    // Update access timestamp for LRU cache management
    it->lastAccessTimeMs = QDateTime::currentMSecsSinceEpoch();
    return it->proxyPath;
}

void ProxyCache::insert(const QString &sourcePath, const QString &proxyPath, int proxyWidth, int proxyHeight)
{
    if (sourcePath.isEmpty() || proxyPath.isEmpty() || !QFile::exists(proxyPath))
        return;

    const QFileInfo info(sourcePath);
    Entry entry;
    entry.proxyPath = proxyPath;
    entry.sourceSize = info.size();
    entry.sourceMtimeMs = info.lastModified().toMSecsSinceEpoch();
    entry.proxyWidth = proxyWidth;
    entry.proxyHeight = proxyHeight;
    entry.lastAccessTimeMs = QDateTime::currentMSecsSinceEpoch();

    QMutexLocker lock(&m_mutex);
    const QString key = info.absoluteFilePath();

    // If an older proxy existed for this source, clean up the old file
    if (m_entries.contains(key) && m_entries[key].proxyPath != proxyPath) {
        QFile::remove(m_entries[key].proxyPath);
    }

    m_entries[key] = entry;
    saveLocked();
}

void ProxyCache::invalidate(const QString &sourcePath)
{
    if (sourcePath.isEmpty())
        return;

    const QString key = QFileInfo(sourcePath).absoluteFilePath();
    QMutexLocker lock(&m_mutex);
    auto it = m_entries.find(key);
    if (it != m_entries.end()) {
        QFile::remove(it->proxyPath);
        m_entries.erase(it);
        saveLocked();
    }
}

void ProxyCache::saveLocked() const
{
    const QString path = indexPath();
    if (path.isEmpty())
        return;

    QJsonObject root;
    root[QStringLiteral("version")] = 1;

    QJsonArray array;
    for (auto it = m_entries.cbegin(); it != m_entries.cend(); ++it) {
        QJsonObject obj;
        obj[QStringLiteral("sourcePath")] = it.key();
        obj[QStringLiteral("proxyPath")] = it.value().proxyPath;
        obj[QStringLiteral("sourceSize")] = it.value().sourceSize;
        obj[QStringLiteral("sourceMtimeMs")] = it.value().sourceMtimeMs;
        obj[QStringLiteral("proxyWidth")] = it.value().proxyWidth;
        obj[QStringLiteral("proxyHeight")] = it.value().proxyHeight;
        obj[QStringLiteral("lastAccessTimeMs")] = it.value().lastAccessTimeMs;
        array.append(obj);
    }
    root[QStringLiteral("entries")] = array;

    QFile file(path + QStringLiteral(".tmp"));
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
        file.close();
        QFile::remove(path);
        file.rename(path);
    }
}

void ProxyCache::load()
{
    const QString path = indexPath();
    if (path.isEmpty() || !QFile::exists(path))
        return;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return;

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isObject())
        return;

    const QJsonArray array = doc.object()[QStringLiteral("entries")].toArray();

    QMutexLocker lock(&m_mutex);
    m_entries.clear();

    for (const QJsonValue &val : array) {
        const QJsonObject obj = val.toObject();
        const QString sourcePath = obj[QStringLiteral("sourcePath")].toString();
        const QString proxyPath = obj[QStringLiteral("proxyPath")].toString();

        if (sourcePath.isEmpty() || proxyPath.isEmpty() || !QFile::exists(proxyPath))
            continue;

        const QFileInfo src(sourcePath);
        if (!src.exists()) {
            QFile::remove(proxyPath);
            continue;
        }

        Entry entry;
        entry.proxyPath = proxyPath;
        entry.sourceSize = obj[QStringLiteral("sourceSize")].toVariant().toLongLong();
        entry.sourceMtimeMs = obj[QStringLiteral("sourceMtimeMs")].toVariant().toLongLong();
        entry.proxyWidth = obj[QStringLiteral("proxyWidth")].toInt();
        entry.proxyHeight = obj[QStringLiteral("proxyHeight")].toInt();
        entry.lastAccessTimeMs = obj[QStringLiteral("lastAccessTimeMs")].toVariant().toLongLong();

        // If source modified, discard stale proxy
        if (entry.sourceMtimeMs != src.lastModified().toMSecsSinceEpoch() || entry.sourceSize != src.size()) {
            QFile::remove(proxyPath);
            continue;
        }

        m_entries[src.absoluteFilePath()] = entry;
    }
}

void ProxyCache::sweep(qint64 maxBytes)
{
    // Clean up any lingering temporary .part files from interrupted renders
    const QString dir = proxyCacheDir();
    if (!dir.isEmpty()) {
        const QDir cacheDir(dir);
        const QStringList strayParts =
            cacheDir.entryList({QStringLiteral("*.part"), QStringLiteral("*.tmp")}, QDir::Files);
        for (const QString &stray : strayParts)
            QFile::remove(cacheDir.filePath(stray));
    }

    QMutexLocker lock(&m_mutex);
    struct CacheItem
    {
        QString sourceKey;
        QString proxyPath;
        qint64 size = 0;
        qint64 lastAccessTimeMs = 0;
    };

    std::vector<CacheItem> items;
    qint64 totalBytes = 0;

    for (auto it = m_entries.begin(); it != m_entries.end();) {
        const QFileInfo proxyInfo(it.value().proxyPath);
        if (!proxyInfo.exists()) {
            it = m_entries.erase(it);
            continue;
        }
        const qint64 sz = proxyInfo.size();
        totalBytes += sz;
        items.push_back({it.key(), it.value().proxyPath, sz, it.value().lastAccessTimeMs});
        ++it;
    }

    if (totalBytes <= maxBytes) {
        saveLocked();
        return;
    }

    // Sort by oldest accessed first
    std::sort(items.begin(), items.end(), [](const CacheItem &a, const CacheItem &b) {
        return a.lastAccessTimeMs < b.lastAccessTimeMs;
    });

    for (const CacheItem &item : items) {
        if (totalBytes <= maxBytes)
            break;
        QFile::remove(item.proxyPath);
        totalBytes -= item.size;
        m_entries.remove(item.sourceKey);
    }

    saveLocked();
}

} // namespace drift
