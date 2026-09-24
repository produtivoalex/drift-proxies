// src/engine/ai/ScriptGenerator.h
// Dark Studio — LLM-powered scriptwriter engine.
// Supports OpenAI GPT-4o-mini, Anthropic Claude Haiku, Google Gemini Flash,
// and a fully offline fallback via llama.cpp REST server (e.g. Phi-3-Mini Q4_K_M).
//
// All network calls are asynchronous; results arrive through Qt signals.
// API keys are stored in QSettings("ai/scriptApiKey", "ai/scriptProvider").
#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QStringList>

namespace drift {

// ── Input ──────────────────────────────────────────────────────────────────

struct ScriptRequest {
    QString topic;           // "Os 5 maiores misterios do Egito"
    QString niche;           // "dark_mystery" | "finance" | "motivation"
                             // "history" | "crime" | "science" | "lifestyle"
    QString format;          // "shorts_60s" | "youtube_8min" | "podcast_20min"
    QString language;        // "pt-BR" | "en-US" | "es-ES" | "de-DE" | "fr-FR"
    QString tone;            // "dramatic" | "calm" | "energetic" | "authoritative"
    int targetDurationSec{60};
};

// ── Output ─────────────────────────────────────────────────────────────────

struct ScriptBlock {
    QString text;            // 2-4 sentences for this block (max 15 words/sentence)
    QStringList brollHints;  // Keywords for auto B-Roll search (e.g. "egypt pyramids")
    QString sfxHint;         // SFX id to play at block boundary ("boom_bass", "whoosh_fast", "")
    double estimatedDurationSec{4.0};
};

struct GeneratedScript {
    QString hook;            // 1 high-impact sentence (0-5 s)
    QList<ScriptBlock> blocks;
    QString callToAction;    // CTA final line ("Salva esse vídeo e segue o canal!")
    QString fullText;        // Concatenated text ready for WizardEngine::generateTimeline()
    QStringList allBrollHints; // Flattened union of all block broll hints
    QStringList allSfxHints;   // Flattened sfx hints for each block boundary
    bool success{false};
    QString error;
};

// ── Engine ─────────────────────────────────────────────────────────────────

class ScriptGenerator : public QObject {
    Q_OBJECT
public:
    explicit ScriptGenerator(QObject *parent = nullptr);
    ~ScriptGenerator() override;

    // Configuration — stored in QSettings; call once on startup and whenever changed.
    // provider: "openai" | "anthropic" | "gemini" | "local"
    void setApiKey(const QString &key, const QString &provider = QStringLiteral("openai"));
    bool hasApiKey() const;
    QString provider() const { return m_provider; }

    // Trigger async generation. Progress signals fire during the request;
    // scriptGenerated() fires when done (success or failure).
    void generateScript(const ScriptRequest &request);

    // Abort any in-flight request (no-op when idle).
    void cancel();

signals:
    // 0.0 → 1.0 during the HTTP round-trip; status is a human-readable description.
    void progressChanged(double fraction, const QString &status);
    void scriptGenerated(const GeneratedScript &result);

private:
    // Prompt assembly — returns the full system+user prompt for the given provider.
    QString buildPrompt(const ScriptRequest &req) const;
    QString nicheInstruction(const QString &niche) const;
    QString formatInstruction(const QString &format, int durationSec) const;

    // Provider-specific HTTP calls
    void callOpenAI(const QString &prompt);
    void callAnthropic(const QString &prompt);
    void callGemini(const QString &prompt);
    // llama.cpp local REST server (default: http://127.0.0.1:8080/completion)
    void callLocalLlama(const QString &prompt);

    // JSON parsing of the LLM response into GeneratedScript
    GeneratedScript parseJsonResponse(const QString &json) const;
    GeneratedScript parseFallbackText(const QString &rawText) const; // graceful degradation

    void handleReplyError(QNetworkReply *reply, GeneratedScript &result) const;

    QNetworkAccessManager m_net;
    QString m_apiKey;
    QString m_provider{QStringLiteral("openai")};
    QNetworkReply *m_activeReply{nullptr};
};

} // namespace drift
