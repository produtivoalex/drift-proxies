// src/engine/ai/ProjectLocalizer.cpp
// Fase 5C — Dublagem Multiidioma (implementação)

#include "ProjectLocalizer.h"
#include "engine/TtsSynthesizer.h"
#include "core/Project.h"
#include "core/Track.h"
#include "core/Clip.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSettings>
#include <QUrlQuery>
#include <QtConcurrent/QtConcurrent>

namespace drift {

// ── Idiomas suportados ───────────────────────────────────────────────────────

QList<LocalizationLanguage> ProjectLocalizer::supportedLanguages()
{
    return {
        { QStringLiteral("en-US"), QStringLiteral("English (US)"),   QStringLiteral("🇺🇸"),
          QStringLiteral("EN-US"), QStringLiteral("en-US-AriaNeural"),   QStringLiteral("en-US") },
        { QStringLiteral("es-ES"), QStringLiteral("Español"),         QStringLiteral("🇪🇸"),
          QStringLiteral("ES"),    QStringLiteral("es-ES-ElviraNeural"), QStringLiteral("es-ES") },
        { QStringLiteral("de-DE"), QStringLiteral("Deutsch"),         QStringLiteral("🇩🇪"),
          QStringLiteral("DE"),    QStringLiteral("de-DE-KatjaNeural"),  QStringLiteral("de-DE") },
        { QStringLiteral("fr-FR"), QStringLiteral("Français"),        QStringLiteral("🇫🇷"),
          QStringLiteral("FR"),    QStringLiteral("fr-FR-DeniseNeural"), QStringLiteral("fr-FR") },
        { QStringLiteral("ja-JP"), QStringLiteral("日本語"),            QStringLiteral("🇯🇵"),
          QStringLiteral("JA"),    QStringLiteral("ja-JP-NanamiNeural"),  QStringLiteral("ja-JP") },
        { QStringLiteral("it-IT"), QStringLiteral("Italiano"),        QStringLiteral("🇮🇹"),
          QStringLiteral("IT"),    QStringLiteral("it-IT-ElsaNeural"),   QStringLiteral("it-IT") },
        { QStringLiteral("ko-KR"), QStringLiteral("한국어"),            QStringLiteral("🇰🇷"),
          QStringLiteral("KO"),    QStringLiteral("ko-KR-SunHiNeural"),  QStringLiteral("ko-KR") },
        { QStringLiteral("pt-PT"), QStringLiteral("Português (PT)"),  QStringLiteral("🇵🇹"),
          QStringLiteral("PT-PT"), QStringLiteral("pt-PT-RaquelNeural"), QStringLiteral("pt-PT") },
    };
}

LocalizationLanguage ProjectLocalizer::languageForCode(const QString &code)
{
    for (const auto &lang : supportedLanguages()) {
        if (lang.code == code) return lang;
    }
    // Fallback: en-US
    return supportedLanguages().first();
}

// ── Text extraction from Project ─────────────────────────────────────────────

QString ProjectLocalizer::extractProjectText(const Project &project)
{
    QStringList texts;
    for (const auto &track : project.tracks()) {
        for (const auto &clip : track.clips) {
            if (!clip.subtitleCues.isEmpty()) {
                for (const auto &cue : clip.subtitleCues)
                    texts << cue.text;
            }
        }
    }
    return texts.join(QStringLiteral(" ")).simplified();
}

// ── Constructor / Destructor ─────────────────────────────────────────────────

ProjectLocalizer::ProjectLocalizer(QObject *parent)
    : QObject(parent)
{
    QSettings s;
    m_deepLKey           = s.value(QStringLiteral("ai/deepLApiKey")).toString();
    m_libreTranslateUrl  = s.value(QStringLiteral("ai/libreTranslateUrl"),
                                    QStringLiteral("https://libretranslate.com")).toString();
}

ProjectLocalizer::~ProjectLocalizer()
{
    cancel();
}

// ── Configuration ────────────────────────────────────────────────────────────

void ProjectLocalizer::setDeepLApiKey(const QString &key)
{
    m_deepLKey = key;
    QSettings().setValue(QStringLiteral("ai/deepLApiKey"), key);
}

void ProjectLocalizer::setLibreTranslateUrl(const QString &url)
{
    m_libreTranslateUrl = url;
    QSettings().setValue(QStringLiteral("ai/libreTranslateUrl"), url);
}

bool ProjectLocalizer::hasTranslationKey() const
{
    return !m_deepLKey.isEmpty() || !m_libreTranslateUrl.isEmpty();
}

void ProjectLocalizer::cancel()
{
    m_cancel = true;
}

// ── Main entry point ─────────────────────────────────────────────────────────

void ProjectLocalizer::localize(const QString &sourceText,
                                 const QString &sourceLang,
                                 const QString &targetLang,
                                 const QString &voiceId,
                                 double speechRate)
{
    if (sourceText.trimmed().isEmpty()) {
        LocalizationResult r;
        r.error = tr("Texto fonte vazio. Adicione legendas ou um roteiro ao projeto primeiro.");
        emit localizationFinished(r);
        return;
    }

    m_cancel = false;
    const LocalizationLanguage langInfo = languageForCode(targetLang);
    const QString effectiveVoice = voiceId.isEmpty() ? langInfo.defaultVoiceId : voiceId;

    emit progressChanged(0.05, tr("Preparando tradução para %1...").arg(langInfo.label));

    // Choose the best available translation engine
    auto onTranslated = [this, targetLang, effectiveVoice, speechRate](
                            const QString &translated, const QString &error) {
        if (m_cancel) return;
        if (!error.isEmpty()) {
            LocalizationResult r;
            r.language = targetLang;
            r.error    = error;
            emit localizationFinished(r);
            return;
        }
        emit progressChanged(0.55, tr("Gerando áudio dublado..."));
        synthesizeAndFinish(translated, targetLang, effectiveVoice, speechRate);
    };

    // Truncate for safety: DeepL free tier = 500k chars/month
    const QString text = sourceText.left(50000);

    if (!m_deepLKey.isEmpty()) {
        translateWithDeepL(text, sourceLang, langInfo.deeplCode, onTranslated);
    } else {
        // LibreTranslate uses BCP-47 language subtag (just the primary, e.g. "pt" not "pt-BR")
        const QString libreSource = sourceLang.section(QLatin1Char('-'), 0, 0).toLower();
        const QString libreTarget = targetLang.section(QLatin1Char('-'), 0, 0).toLower();
        translateWithLibreTranslate(text, libreSource, libreTarget, onTranslated);
    }
}

// ── DeepL API ────────────────────────────────────────────────────────────────

void ProjectLocalizer::translateWithDeepL(
    const QString &text,
    const QString &sourceLang,
    const QString &targetLangDeepL,
    std::function<void(const QString &, const QString &)> callback)
{
    emit progressChanged(0.15, tr("Traduzindo com DeepL..."));

    // DeepL Free API: api-free.deepl.com — Pro: api.deepl.com
    // Auto-detect free vs pro by key suffix (free keys end with ":fx")
    const bool isFree = m_deepLKey.endsWith(QStringLiteral(":fx"));
    const QString host = isFree
        ? QStringLiteral("https://api-free.deepl.com/v2/translate")
        : QStringLiteral("https://api.deepl.com/v2/translate");

    QNetworkRequest req((QUrl(host)));
    req.setHeader(QNetworkRequest::ContentTypeHeader,
                  QStringLiteral("application/x-www-form-urlencoded"));
    req.setRawHeader("Authorization",
                     QStringLiteral("DeepL-Auth-Key %1").arg(m_deepLKey).toUtf8());

    // DeepL accepts source_lang without region subtag (e.g. "PT" not "PT-BR")
    const QString srcCode = sourceLang.section(QLatin1Char('-'), 0, 0).toUpper();

    QUrlQuery body;
    body.addQueryItem(QStringLiteral("text"),        text);
    body.addQueryItem(QStringLiteral("source_lang"), srcCode);
    body.addQueryItem(QStringLiteral("target_lang"), targetLangDeepL);
    body.addQueryItem(QStringLiteral("tag_handling"), QStringLiteral("plain"));
    body.addQueryItem(QStringLiteral("split_sentences"), QStringLiteral("1"));

    QNetworkReply *reply = m_net.post(req, body.query(QUrl::FullyEncoded).toUtf8());

    connect(reply, &QNetworkReply::finished, this, [this, reply, callback]() {
        reply->deleteLater();
        if (m_cancel) return;

        if (reply->error() != QNetworkReply::NoError) {
            callback(QString(), tr("DeepL: %1").arg(reply->errorString()));
            return;
        }

        const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        const QJsonArray translations = doc[QStringLiteral("translations")].toArray();
        if (translations.isEmpty()) {
            callback(QString(), tr("DeepL: resposta vazia ou inválida."));
            return;
        }

        const QString translated = translations.first()[QStringLiteral("text")].toString();
        emit progressChanged(0.50, tr("Tradução concluída!"));
        callback(translated, QString());
    });
}

// ── LibreTranslate API (free fallback) ───────────────────────────────────────

void ProjectLocalizer::translateWithLibreTranslate(
    const QString &text,
    const QString &sourceLang,
    const QString &targetLang,
    std::function<void(const QString &, const QString &)> callback)
{
    emit progressChanged(0.15, tr("Traduzindo com LibreTranslate..."));

    const QString url = m_libreTranslateUrl.trimmed() + QStringLiteral("/translate");

    QNetworkRequest req((QUrl(url)));
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    QJsonObject body;
    body[QStringLiteral("q")]      = text;
    body[QStringLiteral("source")] = sourceLang;
    body[QStringLiteral("target")] = targetLang;
    body[QStringLiteral("format")] = QStringLiteral("text");

    QNetworkReply *reply = m_net.post(req, QJsonDocument(body).toJson(QJsonDocument::Compact));

    connect(reply, &QNetworkReply::finished, this, [this, reply, callback]() {
        reply->deleteLater();
        if (m_cancel) return;

        if (reply->error() != QNetworkReply::NoError) {
            callback(QString(), tr("LibreTranslate: %1").arg(reply->errorString()));
            return;
        }

        const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        const QString translated = doc[QStringLiteral("translatedText")].toString();
        if (translated.isEmpty()) {
            callback(QString(), tr("LibreTranslate: resposta vazia."));
            return;
        }

        emit progressChanged(0.50, tr("Tradução concluída!"));
        callback(translated, QString());
    });
}

// ── TTS Synthesis ────────────────────────────────────────────────────────────

void ProjectLocalizer::synthesizeAndFinish(const QString &translatedText,
                                            const QString &targetLang,
                                            const QString &voiceId,
                                            double speechRate)
{
    // Run TTS on a background thread to avoid blocking the UI
    QFuture<LocalizationResult> future = QtConcurrent::run([=, this]() -> LocalizationResult {
        LocalizationResult result;
        result.language       = targetLang;
        result.translatedText = translatedText;

        if (m_cancel) {
            result.error = tr("Cancelado.");
            return result;
        }

        // Normalise text for TTS in the target language
        const QString normalized = TtsSynthesizer::normalizeTextForTts(translatedText, targetLang);

        // Synthesize
        const TtsSynthesizeResult tts =
            TtsSynthesizer::instance().synthesize(normalized, voiceId, speechRate, 1.0);

        if (!tts.ok) {
            result.error = tr("TTS: %1").arg(tts.error);
            return result;
        }

        result.success          = true;
        result.audioPath        = tts.filePath;
        result.audioDurationSec = tts.durationSeconds;
        result.cues             = tts.cues;
        return result;
    });

    // Watch the future and emit when done
    auto *watcher = new QFutureWatcher<LocalizationResult>(this);
    connect(watcher, &QFutureWatcher<LocalizationResult>::finished, this, [this, watcher]() {
        watcher->deleteLater();
        const LocalizationResult r = watcher->result();
        if (!m_cancel) {
            emit progressChanged(1.0, r.success
                ? tr("Dublagem pronta!")
                : tr("Erro: %1").arg(r.error));
            emit localizationFinished(r);
        }
    });
    watcher->setFuture(future);
}

} // namespace drift
