// src/engine/ai/ScriptGenerator.cpp
// Dark Studio — LLM scriptwriter implementation.

#include "ScriptGenerator.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QSettings>
#include <QUrl>

namespace drift {

// ── Niche and format helpers ────────────────────────────────────────────────

QString ScriptGenerator::nicheInstruction(const QString &niche) const
{
    if (niche == QLatin1String("dark_mystery"))
        return QStringLiteral(
            "Estilo: canal Dark de mistério e curiosidades chocantes do YouTube Brasil. "
            "Tom: sombrio, investigativo, revelador. "
            "Use palavras que despertem medo e curiosidade: 'segredo', 'proibido', 'chocante', 'nunca te contaram'. "
            "Cada frase deve criar tensão crescente.");
    if (niche == QLatin1String("finance"))
        return QStringLiteral(
            "Estilo: canal de finanças pessoais e investimentos para brasileiros. "
            "Tom: direto, autoritário, prático. "
            "Use números concretos, percentuais e exemplos reais. "
            "Foco em transformação financeira rápida e realizável.");
    if (niche == QLatin1String("motivation"))
        return QStringLiteral(
            "Estilo: canal motivacional de alto impacto para jovens brasileiros. "
            "Tom: energético, desafiador, empoderador. "
            "Use imperativos e frases curtas que inspirem ação imediata.");
    if (niche == QLatin1String("history"))
        return QStringLiteral(
            "Estilo: canal de história e cultura com narrativa cinematográfica. "
            "Tom: épico, narrado, apaixonante. "
            "Traga fatos obscuros e pouco conhecidos que surpreendam o espectador.");
    if (niche == QLatin1String("crime"))
        return QStringLiteral(
            "Estilo: canal true crime brasileiro. "
            "Tom: jornalístico, tenso, investigativo. "
            "Narre os fatos com detalhes que prendam a atenção do início ao fim.");
    if (niche == QLatin1String("science"))
        return QStringLiteral(
            "Estilo: canal de ciência e tecnologia para público jovem brasileiro. "
            "Tom: fascinante, acessível, surpreendente. "
            "Transforme conceitos complexos em revelações simples e impactantes.");
    // lifestyle / default
    return QStringLiteral(
        "Estilo: canal de lifestyle e comportamento para brasileiros modernos. "
        "Tom: autêntico, próximo, conversacional.");
}

QString ScriptGenerator::formatInstruction(const QString &format, int durationSec) const
{
    if (format == QLatin1String("shorts_60s") || durationSec <= 75)
        return QStringLiteral(
            "Formato: YouTube Shorts / TikTok / Reels de até 60 segundos. "
            "Estrutura OBRIGATÓRIA: gancho impactante de 1 frase (0-5s) + 4-6 blocos curtos de 2-3 frases + CTA de 1 frase. "
            "MÁXIMO 12 palavras por frase. Sem enrolação.");
    if (format == QLatin1String("youtube_8min") || durationSec <= 600)
        return QStringLiteral(
            "Formato: vídeo YouTube de 7-9 minutos. "
            "Estrutura: gancho (0-15s) + desenvolvimento em 6-8 blocos temáticos de 4-5 frases + CTA. "
            "Cada bloco deve ter uma revelação ou virada narrativa.");
    // podcast / long form
    return QStringLiteral(
        "Formato: podcast ou vídeo longo de 15-25 minutos. "
        "Estrutura: abertura envolvente (1-2 min) + 8-12 segmentos bem estruturados com exemplos + encerramento reflexivo. "
        "Aprofunde cada ponto com histórias e dados concretos.");
}

// ── Prompt builder ──────────────────────────────────────────────────────────

QString ScriptGenerator::buildPrompt(const ScriptRequest &req) const
{
    const QString nicheStr = nicheInstruction(req.niche);
    const QString formatStr = formatInstruction(req.format, req.targetDurationSec);
    const QString langLabel = req.language == QLatin1String("pt-BR") ? QStringLiteral("português brasileiro")
                            : req.language == QLatin1String("en-US") ? QStringLiteral("American English")
                            : req.language == QLatin1String("es-ES") ? QStringLiteral("español")
                            : req.language == QLatin1String("de-DE") ? QStringLiteral("Deutsch")
                            : req.language == QLatin1String("fr-FR") ? QStringLiteral("français")
                            : req.language;

    return QStringLiteral(
        "Você é um roteirista viral de elite especializado em conteúdo para redes sociais. "
        "Seu roteiro deve ser escrito em %1.\n\n"
        "%2\n\n"
        "%3\n\n"
        "Tema do vídeo: \"%4\"\n\n"
        "INSTRUÇÕES CRÍTICAS:\n"
        "1. Retorne APENAS um JSON válido, sem markdown, sem explicações extras.\n"
        "2. Estrutura JSON obrigatória:\n"
        "{\n"
        "  \"hook\": \"Uma frase de abertura CHOCANTE que para o scroll imediatamente\",\n"
        "  \"blocks\": [\n"
        "    {\n"
        "      \"text\": \"2-3 frases do bloco\",\n"
        "      \"broll_hints\": [\"keyword1 para busca de video\", \"keyword2\"],\n"
        "      \"sfx_hint\": \"boom_bass\"\n"
        "    }\n"
        "  ],\n"
        "  \"cta\": \"Frase final de call to action para engajamento\"\n"
        "}\n\n"
        "Valores possíveis para sfx_hint: \"boom_bass\", \"whoosh_fast\", \"whoosh_deep\", \"glitch_rise\", \"\" (vazio = sem som).\n"
        "Os broll_hints devem ser palavras-chave em inglês para buscar no Pexels/Pixabay.\n"
        "ESCREVA AGORA o roteiro viral sobre o tema dado:"
    ).arg(langLabel, nicheStr, formatStr, req.topic);
}

// ── Constructor / destructor ────────────────────────────────────────────────

ScriptGenerator::ScriptGenerator(QObject *parent)
    : QObject(parent)
{
    QSettings s;
    m_apiKey  = s.value(QStringLiteral("ai/scriptApiKey")).toString();
    m_provider = s.value(QStringLiteral("ai/scriptProvider"),
                          QStringLiteral("openai")).toString();
}

ScriptGenerator::~ScriptGenerator()
{
    cancel();
}

// ── Public interface ────────────────────────────────────────────────────────

void ScriptGenerator::setApiKey(const QString &key, const QString &provider)
{
    m_apiKey   = key;
    m_provider = provider;
    QSettings s;
    s.setValue(QStringLiteral("ai/scriptApiKey"),   key);
    s.setValue(QStringLiteral("ai/scriptProvider"), provider);
}

bool ScriptGenerator::hasApiKey() const
{
    return m_provider == QLatin1String("local") || !m_apiKey.isEmpty();
}

void ScriptGenerator::cancel()
{
    if (m_activeReply) {
        m_activeReply->abort();
        m_activeReply = nullptr;
    }
}

void ScriptGenerator::generateScript(const ScriptRequest &request)
{
    cancel();
    const QString prompt = buildPrompt(request);

    emit progressChanged(0.05, tr("Conectando ao roteirista..."));

    if (m_provider == QLatin1String("anthropic"))
        callAnthropic(prompt);
    else if (m_provider == QLatin1String("gemini"))
        callGemini(prompt);
    else if (m_provider == QLatin1String("local"))
        callLocalLlama(prompt);
    else
        callOpenAI(prompt); // default
}

// ── Provider implementations ────────────────────────────────────────────────

void ScriptGenerator::callOpenAI(const QString &prompt)
{
    QNetworkRequest req(QUrl(QStringLiteral("https://api.openai.com/v1/chat/completions")));
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    req.setRawHeader("Authorization", QStringLiteral("Bearer %1").arg(m_apiKey).toUtf8());

    QJsonObject body{
        {QStringLiteral("model"),       QStringLiteral("gpt-4o-mini")},
        {QStringLiteral("temperature"), 0.85},
        {QStringLiteral("messages"),    QJsonArray{
            QJsonObject{{QStringLiteral("role"), QStringLiteral("user")},
                        {QStringLiteral("content"), prompt}}
        }}
    };

    m_activeReply = m_net.post(req, QJsonDocument(body).toJson(QJsonDocument::Compact));
    emit progressChanged(0.2, tr("Gerando roteiro com IA..."));

    connect(m_activeReply, &QNetworkReply::finished, this, [this] {
        auto *reply = qobject_cast<QNetworkReply *>(sender());
        if (!reply) return;
        reply->deleteLater();
        m_activeReply = nullptr;

        if (reply->error() != QNetworkReply::NoError) {
            GeneratedScript r;
            handleReplyError(reply, r);
            emit scriptGenerated(r);
            return;
        }
        const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        const QString text = doc[QStringLiteral("choices")][0][QStringLiteral("message")]
                                 [QStringLiteral("content")].toString().trimmed();
        emit progressChanged(0.9, tr("Processando roteiro..."));
        emit scriptGenerated(parseJsonResponse(text));
    });
}

void ScriptGenerator::callAnthropic(const QString &prompt)
{
    QNetworkRequest req(QUrl(QStringLiteral("https://api.anthropic.com/v1/messages")));
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    req.setRawHeader("x-api-key",         m_apiKey.toUtf8());
    req.setRawHeader("anthropic-version", "2023-06-01");

    QJsonObject body{
        {QStringLiteral("model"),      QStringLiteral("claude-haiku-20240307")},
        {QStringLiteral("max_tokens"), 2048},
        {QStringLiteral("messages"),   QJsonArray{
            QJsonObject{{QStringLiteral("role"),    QStringLiteral("user")},
                        {QStringLiteral("content"), prompt}}
        }}
    };

    m_activeReply = m_net.post(req, QJsonDocument(body).toJson(QJsonDocument::Compact));
    emit progressChanged(0.2, tr("Gerando roteiro com IA..."));

    connect(m_activeReply, &QNetworkReply::finished, this, [this] {
        auto *reply = qobject_cast<QNetworkReply *>(sender());
        if (!reply) return;
        reply->deleteLater();
        m_activeReply = nullptr;

        if (reply->error() != QNetworkReply::NoError) {
            GeneratedScript r;
            handleReplyError(reply, r);
            emit scriptGenerated(r);
            return;
        }
        const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        const QString text = doc[QStringLiteral("content")][0][QStringLiteral("text")]
                                 .toString().trimmed();
        emit progressChanged(0.9, tr("Processando roteiro..."));
        emit scriptGenerated(parseJsonResponse(text));
    });
}

void ScriptGenerator::callGemini(const QString &prompt)
{
    const QString url = QStringLiteral(
        "https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent?key=%1"
    ).arg(m_apiKey);

    QNetworkRequest req((QUrl(url)));
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    QJsonObject body{
        {QStringLiteral("contents"), QJsonArray{
            QJsonObject{{QStringLiteral("parts"), QJsonArray{
                QJsonObject{{QStringLiteral("text"), prompt}}
            }}}
        }}
    };

    m_activeReply = m_net.post(req, QJsonDocument(body).toJson(QJsonDocument::Compact));
    emit progressChanged(0.2, tr("Gerando roteiro com IA..."));

    connect(m_activeReply, &QNetworkReply::finished, this, [this] {
        auto *reply = qobject_cast<QNetworkReply *>(sender());
        if (!reply) return;
        reply->deleteLater();
        m_activeReply = nullptr;

        if (reply->error() != QNetworkReply::NoError) {
            GeneratedScript r;
            handleReplyError(reply, r);
            emit scriptGenerated(r);
            return;
        }
        const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        const QString text = doc[QStringLiteral("candidates")][0]
                                 [QStringLiteral("content")]
                                 [QStringLiteral("parts")][0]
                                 [QStringLiteral("text")].toString().trimmed();
        emit progressChanged(0.9, tr("Processando roteiro..."));
        emit scriptGenerated(parseJsonResponse(text));
    });
}

void ScriptGenerator::callLocalLlama(const QString &prompt)
{
    // Assumes llama.cpp server running at localhost:8080 (e.g. phi-3-mini Q4_K_M)
    QNetworkRequest req(QUrl(QStringLiteral("http://127.0.0.1:8080/completion")));
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    QJsonObject body{
        {QStringLiteral("prompt"),      prompt},
        {QStringLiteral("n_predict"),   1024},
        {QStringLiteral("temperature"), 0.85},
        {QStringLiteral("stop"),        QJsonArray{QStringLiteral("</s>")}}
    };

    m_activeReply = m_net.post(req, QJsonDocument(body).toJson(QJsonDocument::Compact));
    emit progressChanged(0.2, tr("Gerando roteiro com IA local..."));

    connect(m_activeReply, &QNetworkReply::finished, this, [this] {
        auto *reply = qobject_cast<QNetworkReply *>(sender());
        if (!reply) return;
        reply->deleteLater();
        m_activeReply = nullptr;

        if (reply->error() != QNetworkReply::NoError) {
            GeneratedScript r;
            r.error = tr("IA local não encontrada. Inicie o servidor llama.cpp na porta 8080 "
                         "ou configure uma chave de API em Configurações → IA.");
            emit scriptGenerated(r);
            return;
        }
        const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        const QString text = doc[QStringLiteral("content")].toString().trimmed();
        emit progressChanged(0.9, tr("Processando roteiro..."));
        emit scriptGenerated(parseJsonResponse(text));
    });
}

// ── JSON parsing ────────────────────────────────────────────────────────────

GeneratedScript ScriptGenerator::parseJsonResponse(const QString &json) const
{
    GeneratedScript result;

    // Strip markdown code fences if the LLM wrapped in ```json ... ```
    QString clean = json;
    if (clean.startsWith(QLatin1String("```"))) {
        const int start = clean.indexOf(QLatin1Char('\n')) + 1;
        const int end   = clean.lastIndexOf(QLatin1String("```"));
        if (start > 0 && end > start)
            clean = clean.mid(start, end - start).trimmed();
    }

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(clean.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        // JSON parse failed — try to salvage a plain-text script
        return parseFallbackText(json);
    }

    const QJsonObject obj = doc.object();
    result.hook = obj[QStringLiteral("hook")].toString();
    result.callToAction = obj[QStringLiteral("cta")].toString();

    QStringList fullParts;
    fullParts << result.hook;

    const QJsonArray blocksArr = obj[QStringLiteral("blocks")].toArray();
    for (const QJsonValue &v : blocksArr) {
        const QJsonObject b = v.toObject();
        ScriptBlock block;
        block.text = b[QStringLiteral("text")].toString();
        block.sfxHint = b[QStringLiteral("sfx_hint")].toString();

        const QJsonArray hints = b[QStringLiteral("broll_hints")].toArray();
        for (const QJsonValue &h : hints)
            block.brollHints << h.toString();

        // Rough duration estimate: ~130 words/min narration speed
        const int wordCount = block.text.split(QLatin1Char(' '), Qt::SkipEmptyParts).size();
        block.estimatedDurationSec = qMax(2.0, wordCount / 2.2);

        result.blocks << block;
        result.allBrollHints << block.brollHints;
        if (!block.sfxHint.isEmpty())
            result.allSfxHints << block.sfxHint;

        fullParts << block.text;
    }

    fullParts << result.callToAction;
    result.fullText = fullParts.join(QStringLiteral("\n\n"));
    result.success  = !result.hook.isEmpty() && !result.blocks.isEmpty();
    if (!result.success)
        result.error = tr("A IA retornou um roteiro vazio. Tente novamente com outro tema.");

    return result;
}

GeneratedScript ScriptGenerator::parseFallbackText(const QString &rawText) const
{
    // Graceful degradation: treat each paragraph as a block
    GeneratedScript result;
    const QStringList paragraphs = rawText.split(QStringLiteral("\n\n"), Qt::SkipEmptyParts);
    if (paragraphs.isEmpty()) {
        result.error = tr("A IA retornou uma resposta inesperada. Tente novamente.");
        return result;
    }

    result.hook = paragraphs.first().trimmed();
    for (int i = 1; i < paragraphs.size() - 1; ++i) {
        ScriptBlock b;
        b.text = paragraphs[i].trimmed();
        result.blocks << b;
    }
    if (paragraphs.size() > 1)
        result.callToAction = paragraphs.last().trimmed();

    result.fullText = rawText;
    result.success  = true;
    return result;
}

void ScriptGenerator::handleReplyError(QNetworkReply *reply, GeneratedScript &result) const
{
    const int httpCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (httpCode == 401)
        result.error = tr("API key inválida. Verifique em Configurações → IA.");
    else if (httpCode == 429)
        result.error = tr("Limite de requisições atingido. Aguarde um momento e tente novamente.");
    else
        result.error = tr("Erro de rede (%1): %2").arg(httpCode).arg(reply->errorString());
}

} // namespace drift
