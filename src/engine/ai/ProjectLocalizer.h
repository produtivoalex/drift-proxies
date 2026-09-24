// src/engine/ai/ProjectLocalizer.h
// Fase 5C — Dublagem Multiidioma
//
// Fluxo completo:
//   1. Extrai o texto do projeto (legendas existentes ou roteiro bruto)
//   2. Traduz com DeepL API (ou LibreTranslate como fallback gratuito)
//   3. Normaliza o texto traduzido para TTS (números, abreviações, etc.)
//   4. Gera áudio dublado com TtsSynthesizer (voz neural selecionada por idioma)
//   5. Insere nova faixa de áudio na timeline + legendas traduzidas
//
// Idiomas suportados: pt-BR, en-US, es-ES, de-DE, fr-FR, ja-JP, it-IT, ko-KR
// APIs de tradução: DeepL (pago, alta qualidade) / LibreTranslate (gratuito)
#pragma once

#include "core/SubtitleCue.h"

#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QSettings>
#include <atomic>

namespace drift {

class Project;

// ── Idioma alvo com voz TTS recomendada ─────────────────────────────────────

struct LocalizationLanguage {
    QString code;           // BCP-47: "en-US", "es-ES", "de-DE", "fr-FR", "ja-JP"
    QString label;          // "English (US)", "Español", …
    QString flag;           // emoji
    QString deeplCode;      // código para DeepL: "EN-US", "ES", "DE", "FR", "JA"
    QString defaultVoiceId; // voz TTS preferida p/ esse idioma
    QString ttsLang;        // código SSML/TTS: "en-US", "es-ES", …
};

// ── Resultado da localização ─────────────────────────────────────────────────

struct LocalizationResult {
    bool success = false;
    QString language;        // código do idioma
    QString translatedText;  // texto completo traduzido
    QString audioPath;       // caminho do WAV/MP3 gerado
    QList<SubtitleCue> cues; // legendas sincronizadas no idioma alvo
    double audioDurationSec = 0.0;
    QString error;
};

// ── Engine Principal ─────────────────────────────────────────────────────────

class ProjectLocalizer : public QObject {
    Q_OBJECT
public:
    explicit ProjectLocalizer(QObject *parent = nullptr);
    ~ProjectLocalizer() override;

    // Configuração de API keys
    void setDeepLApiKey(const QString &key);
    void setLibreTranslateUrl(const QString &url); // e.g. "https://libretranslate.com"
    bool hasTranslationKey() const;

    // Localiza o projeto para um idioma específico.
    // `sourceText`: texto original (roteiro bruto ou legendas concatenadas)
    // `sourceLang`: idioma original (e.g. "pt-BR")
    // `targetLang`: idioma alvo (e.g. "en-US")
    // `voiceId`: voz TTS (vazio = usa a voz padrão do idioma)
    // `speechRate`: velocidade de fala (0.75 – 1.5)
    void localize(const QString &sourceText,
                  const QString &sourceLang,
                  const QString &targetLang,
                  const QString &voiceId = QString(),
                  double speechRate = 1.0);

    // Cancela qualquer operação em andamento
    void cancel();

    // Idiomas suportados com metadados
    static QList<LocalizationLanguage> supportedLanguages();
    static LocalizationLanguage languageForCode(const QString &code);

    // Extrai o texto de legenda de um projeto para usar como fonte de tradução
    static QString extractProjectText(const Project &project);

signals:
    void progressChanged(double fraction, const QString &status);
    void localizationFinished(const drift::LocalizationResult &result);

private:
    // Step 1: Translate via DeepL
    void translateWithDeepL(const QString &text,
                             const QString &sourceLang,
                             const QString &targetLangDeepL,
                             std::function<void(const QString &translated, const QString &error)> callback);

    // Step 2: Translate via LibreTranslate (free fallback)
    void translateWithLibreTranslate(const QString &text,
                                      const QString &sourceLang,
                                      const QString &targetLang,
                                      std::function<void(const QString &translated, const QString &error)> callback);

    // Step 3: TTS synthesis + cue generation
    void synthesizeAndFinish(const QString &translatedText,
                              const QString &targetLang,
                              const QString &voiceId,
                              double speechRate);

    QNetworkAccessManager m_net;
    QString m_deepLKey;
    QString m_libreTranslateUrl;
    std::atomic<bool> m_cancel{false};
};

} // namespace drift
