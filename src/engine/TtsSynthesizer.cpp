#include "TtsSynthesizer.h"
#include "core/Time.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QUuid>
#include <QDebug>

#include <algorithm>
#include <cmath>

namespace drift {

TtsSynthesizer &TtsSynthesizer::instance()
{
    static TtsSynthesizer s_instance;
    return s_instance;
}

TtsSynthesizer::TtsSynthesizer()
{
    QDir dir(ttsOutputDir());
    if (!dir.exists()) {
        dir.mkpath(QStringLiteral("."));
    }
}

QString TtsSynthesizer::ttsOutputDir() const
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!base.isEmpty()) {
        return base + QStringLiteral("/tts");
    }
    return QDir::tempPath() + QStringLiteral("/drift_tts");
}

QList<TtsVoiceInfo> TtsSynthesizer::availableVoices()
{
    QList<TtsVoiceInfo> voices;

    // --- AS 5 VOZES GRATUITAS MAIS REALISTAS E HUMANIZADAS DO BRASIL ---
    // 1. Thalita - Viral, jovem, descontraída (TikTok / Reels)
    TtsVoiceInfo vThalita;
    vThalita.id = QStringLiteral("pt-BR-ThalitaMultilingualNeural");
    vThalita.name = QStringLiteral("Thalita (🔥 Viral & Espontânea)");
    vThalita.lang = QStringLiteral("pt-BR");
    vThalita.gender = QStringLiteral("Feminino");
    vThalita.vibeTag = QStringLiteral("🔥 Viral & Espontânea");
    vThalita.description = QStringLiteral("Jovem, enérgica e descontraída. A voz nº 1 para TikTok, Reels e vídeos curtos no Brasil. Zero robótica.");
    vThalita.isFeatured = true;
    vThalita.isNeural = true;
    vThalita.defaultRate = 1.05;
    vThalita.defaultPitch = 1.0;
    voices.append(vThalita);

    // 2. Antonio - Épico, documentário, narrador de canais Dark
    TtsVoiceInfo vAntonio;
    vAntonio.id = QStringLiteral("pt-BR-AntonioNeural");
    vAntonio.name = QStringLiteral("Antonio (🎙️ Épico & Documentário)");
    vAntonio.lang = QStringLiteral("pt-BR");
    vAntonio.gender = QStringLiteral("Masculino");
    vAntonio.vibeTag = QStringLiteral("🎙️ Épico & Documentário");
    vAntonio.description = QStringLiteral("Tom encorpado, profundo e cinematográfico. A voz clássica de canais Dark, histórias, mistério e narrações épicas.");
    vAntonio.isFeatured = true;
    vAntonio.isNeural = true;
    vAntonio.defaultRate = 1.0;
    vAntonio.defaultPitch = 1.0;
    voices.append(vAntonio);

    // 3. Francisca - Storyteller, elegante, humana e expressiva
    TtsVoiceInfo vFrancisca;
    vFrancisca.id = QStringLiteral("pt-BR-FranciscaNeural");
    vFrancisca.name = QStringLiteral("Francisca (✨ Storyteller & Expressiva)");
    vFrancisca.lang = QStringLiteral("pt-BR");
    vFrancisca.gender = QStringLiteral("Feminino");
    vFrancisca.vibeTag = QStringLiteral("✨ Storyteller & Expressiva");
    vFrancisca.description = QStringLiteral("Voz calorosa, inteligente e empática. Excelente para tutoriais explicativos, reviews e roteiros longos.");
    vFrancisca.isFeatured = true;
    vFrancisca.isNeural = true;
    vFrancisca.defaultRate = 1.0;
    vFrancisca.defaultPitch = 1.0;
    voices.append(vFrancisca);

    // 4. Fabio - Dinâmico, jovem e ritmo rápido para Tech & Curiosidades
    TtsVoiceInfo vFabio;
    vFabio.id = QStringLiteral("en-US-AndrewMultilingualNeural");
    vFabio.name = QStringLiteral("Fabio (⚡ Tech & Dinâmico)");
    vFabio.lang = QStringLiteral("pt-BR");
    vFabio.gender = QStringLiteral("Masculino");
    vFabio.vibeTag = QStringLiteral("⚡ Tech & Dinâmico");
    vFabio.description = QStringLiteral("Voz jovem, ágil e vibrante. Ideal para vídeos de tecnologia, curiosidades e alta retenção.");
    vFabio.isFeatured = true;
    vFabio.isNeural = true;
    vFabio.defaultRate = 1.05;
    vFabio.defaultPitch = 1.0;
    voices.append(vFabio);

    // 5. Yara - Descolada, podcaster, lifestyle e tom conversacional
    TtsVoiceInfo vYara;
    vYara.id = QStringLiteral("en-US-AvaMultilingualNeural");
    vYara.name = QStringLiteral("Yara (💬 Autêntica & Lifestyle)");
    vYara.lang = QStringLiteral("pt-BR");
    vYara.gender = QStringLiteral("Feminino");
    vYara.vibeTag = QStringLiteral("💬 Autêntica & Lifestyle");
    vYara.description = QStringLiteral("Tom descolado e moderno, estilo bate-papo e podcaster. Perfeita para vlogs, lifestyle e conselhos.");
    vYara.isFeatured = true;
    vYara.isNeural = true;
    vYara.defaultRate = 1.0;
    vYara.defaultPitch = 1.0;
    voices.append(vYara);

#if defined(Q_OS_MACOS)
    QProcess proc;
    proc.start(QStringLiteral("say"), {QStringLiteral("-v"), QStringLiteral("?")});
    if (proc.waitForFinished(3000)) {
        const QString out = QString::fromUtf8(proc.readAllStandardOutput());
        const QStringList lines = out.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        for (const QString &line : lines) {
            const QString trimmed = line.trimmed();
            if (!trimmed.isEmpty()) {
                const QString voiceName = trimmed.section(QLatin1Char(' '), 0, 0);
                const QString langCode = trimmed.section(QLatin1Char(' '), 1, 1);
                TtsVoiceInfo info;
                info.id = voiceName;
                info.name = voiceName;
                info.lang = langCode;
                info.vibeTag = QStringLiteral("Voz macOS");
                voices.append(info);
            }
        }
    }
#else
    TtsVoiceInfo v;
    v.id = QStringLiteral("pt-br");
    v.name = QStringLiteral("Voz em Português (espeak-ng)");
    v.lang = QStringLiteral("pt-BR");
    v.gender = QStringLiteral("Neutro");
    v.vibeTag = QStringLiteral("Linux TTS");
    voices.append(v);
#endif

    // Check for offline Piper ONNX neural models in models directory
    const QString modelsDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/models/tts");
    QDir mDir(modelsDir);
    if (mDir.exists()) {
        const QStringList files = mDir.entryList({QStringLiteral("*.onnx")}, QDir::Files);
        for (const QString &f : files) {
            TtsVoiceInfo nv;
            nv.id = mDir.filePath(f);
            nv.name = f.section(QLatin1Char('.'), 0, 0) + QStringLiteral(" (Piper ONNX)");
            nv.lang = f.startsWith(QStringLiteral("pt_BR"), Qt::CaseInsensitive) ? QStringLiteral("pt-BR") : QStringLiteral("en-US");
            nv.gender = QStringLiteral("Neural");
            nv.vibeTag = QStringLiteral("🧠 Neural Offline");
            nv.isNeural = true;
            voices.insert(5, nv); // insert right below the top 5 featured
        }
    }

    return voices;
}

double TtsSynthesizer::measureWavDuration(const QString &wavPath) const
{
    QFile file(wavPath);
    if (!file.open(QIODevice::ReadOnly))
        return 0.0;

    QByteArray header = file.read(64);
    if (header.size() < 44)
        return 0.0;

    // RIFF header validation
    if (!header.startsWith("RIFF") || header.mid(8, 4) != "WAVE")
        return 0.0;

    quint32 bytesPerSec = *reinterpret_cast<const quint32 *>(header.constData() + 28);
    quint32 dataSize = file.size() > 44 ? static_cast<quint32>(file.size() - 44) : 0;

    // Check for "data" chunk
    int dataIdx = header.indexOf("data");
    if (dataIdx >= 0 && dataIdx + 8 <= header.size()) {
        dataSize = *reinterpret_cast<const quint32 *>(header.constData() + dataIdx + 4);
    }

    if (bytesPerSec > 0 && dataSize > 0) {
        return static_cast<double>(dataSize) / static_cast<double>(bytesPerSec);
    }

    return 0.0;
}

QList<SubtitleCue> TtsSynthesizer::generateCuesForText(const QString &text, double totalDurationSec)
{
    QList<SubtitleCue> cues;
    if (text.trimmed().isEmpty() || totalDurationSec <= 0.05)
        return cues;

    // Split text into natural breath/sentence segments
    // Delimiters: . ! ? ; or newlines
    static const QRegularExpression sentenceRx(QStringLiteral(R"((?<=[.!?;\n])\s+)"));
    QStringList sentences = text.split(sentenceRx, Qt::SkipEmptyParts);

    // If sentences are very long, further split by commas or clause markers
    QStringList chunks;
    for (const QString &s : sentences) {
        const QString st = s.trimmed();
        if (st.isEmpty())
            continue;

        if (st.length() > 60 && st.contains(QLatin1Char(','))) {
            const QStringList subChunks = st.split(QRegularExpression(QStringLiteral(R"((?<=,)\s+)")), Qt::SkipEmptyParts);
            for (const QString &sc : subChunks) {
                if (!sc.trimmed().isEmpty())
                    chunks.append(sc.trimmed());
            }
        } else {
            chunks.append(st);
        }
    }

    if (chunks.isEmpty()) {
        chunks.append(text.trimmed());
    }

    // Count words per chunk to distribute duration accurately
    QList<int> wordCounts;
    int totalWords = 0;
    for (const QString &chunk : chunks) {
        const int words = std::max<int>(1, static_cast<int>(chunk.split(QRegularExpression(QStringLiteral(R"(\s+)")), Qt::SkipEmptyParts).size()));
        wordCounts.append(words);
        totalWords += words;
    }

    if (totalWords == 0)
        totalWords = 1;

    double currentTime = 0.0;
    for (int i = 0; i < chunks.size(); ++i) {
        const double proportion = static_cast<double>(wordCounts[i]) / static_cast<double>(totalWords);
        double chunkDuration = proportion * totalDurationSec;

        // Ensure duration doesn't overshoot
        if (i == chunks.size() - 1) {
            chunkDuration = std::max<double>(0.2, totalDurationSec - currentTime);
        }

        SubtitleCue cue;
        cue.startUs = secondsToUs(currentTime);
        cue.endUs = secondsToUs(currentTime + chunkDuration);
        cue.text = chunks[i];

        cues.append(cue);
        currentTime += chunkDuration;
    }

    return cues;
}

namespace {

QString convertHundreds(qint64 n)
{
    static const QStringList units = {
        QStringLiteral(""), QStringLiteral("um"), QStringLiteral("dois"), QStringLiteral("três"),
        QStringLiteral("quatro"), QStringLiteral("cinco"), QStringLiteral("seis"), QStringLiteral("sete"),
        QStringLiteral("oito"), QStringLiteral("nove"), QStringLiteral("dez"), QStringLiteral("onze"),
        QStringLiteral("doze"), QStringLiteral("treze"), QStringLiteral("quatorze"), QStringLiteral("quinze"),
        QStringLiteral("dezesseis"), QStringLiteral("dezessete"), QStringLiteral("dezoito"), QStringLiteral("dezenove")
    };
    static const QStringList tens = {
        QStringLiteral(""), QStringLiteral(""), QStringLiteral("vinte"), QStringLiteral("trinta"),
        QStringLiteral("quarenta"), QStringLiteral("cinquenta"), QStringLiteral("sessenta"),
        QStringLiteral("setenta"), QStringLiteral("oitenta"), QStringLiteral("noventa")
    };
    static const QStringList hundreds = {
        QStringLiteral(""), QStringLiteral("cento"), QStringLiteral("duzentos"), QStringLiteral("trezentos"),
        QStringLiteral("quatrocentos"), QStringLiteral("quinhentos"), QStringLiteral("seiscentos"),
        QStringLiteral("setecentos"), QStringLiteral("oitocentos"), QStringLiteral("novecentos")
    };

    if (n == 0)
        return QString();
    if (n == 100)
        return QStringLiteral("cem");

    QStringList parts;
    const int c = static_cast<int>(n / 100);
    const int remainder = static_cast<int>(n % 100);

    if (c > 0 && c < hundreds.size())
        parts.append(hundreds.at(c));

    if (remainder > 0) {
        if (remainder < 20) {
            parts.append(units.at(remainder));
        } else {
            const int d = remainder / 10;
            const int u = remainder % 10;
            if (d < tens.size())
                parts.append(tens.at(d));
            if (u > 0 && u < units.size())
                parts.append(units.at(u));
        }
    }

    return parts.join(QStringLiteral(" e "));
}

QString numberToPortugueseWords(qint64 n)
{
    if (n == 0)
        return QStringLiteral("zero");
    if (n < 0)
        return QStringLiteral("menos ") + numberToPortugueseWords(-n);

    if (n < 1000)
        return convertHundreds(n);

    QStringList parts;

    const qint64 billions = n / 1000000000;
    n %= 1000000000;
    if (billions > 0) {
        if (billions == 1)
            parts.append(QStringLiteral("um bilhão"));
        else
            parts.append(convertHundreds(billions) + QStringLiteral(" bilhões"));
    }

    const qint64 millions = n / 1000000;
    n %= 1000000;
    if (millions > 0) {
        if (millions == 1)
            parts.append(QStringLiteral("um milhão"));
        else
            parts.append(convertHundreds(millions) + QStringLiteral(" milhões"));
    }

    const qint64 thousands = n / 1000;
    n %= 1000;
    if (thousands > 0) {
        if (thousands == 1)
            parts.append(QStringLiteral("mil"));
        else
            parts.append(convertHundreds(thousands) + QStringLiteral(" mil"));
    }

    if (n > 0) {
        parts.append(convertHundreds(n));
    }

    return parts.join(QStringLiteral(" e "));
}

QString ordinalToPortuguese(int n, bool feminine)
{
    static const QStringList ordM = {
        QStringLiteral(""), QStringLiteral("primeiro"), QStringLiteral("segundo"), QStringLiteral("terceiro"),
        QStringLiteral("quarto"), QStringLiteral("quinto"), QStringLiteral("sexto"), QStringLiteral("sétimo"),
        QStringLiteral("oitavo"), QStringLiteral("nono"), QStringLiteral("décimo")
    };
    static const QStringList ordF = {
        QStringLiteral(""), QStringLiteral("primeira"), QStringLiteral("segunda"), QStringLiteral("terceira"),
        QStringLiteral("quarta"), QStringLiteral("quinta"), QStringLiteral("sexta"), QStringLiteral("sétima"),
        QStringLiteral("oitava"), QStringLiteral("nona"), QStringLiteral("décima")
    };

    if (n >= 1 && n <= 10)
        return feminine ? ordF.at(n) : ordM.at(n);
    return numberToPortugueseWords(n);
}

static const QStringList kMonthsPt = {
    QStringLiteral(""), QStringLiteral("janeiro"), QStringLiteral("fevereiro"), QStringLiteral("março"),
    QStringLiteral("abril"), QStringLiteral("maio"), QStringLiteral("junho"), QStringLiteral("julho"),
    QStringLiteral("agosto"), QStringLiteral("setembro"), QStringLiteral("outubro"), QStringLiteral("novembro"),
    QStringLiteral("dezembro")
};

template <typename Functor>
static QString replaceAll(const QString &input, const QRegularExpression &re, Functor &&replacer)
{
    QString result;
    int lastPos = 0;
    auto it = re.globalMatch(input);
    while (it.hasNext()) {
        auto match = it.next();
        result.append(input.mid(lastPos, match.capturedStart() - lastPos));
        result.append(replacer(match));
        lastPos = match.capturedEnd();
    }
    result.append(input.mid(lastPos));
    return result;
}

} // namespace

QString TtsSynthesizer::normalizeTextForTts(const QString &text, const QString &lang)
{
    Q_UNUSED(lang);
    if (text.trimmed().isEmpty())
        return text;

    QString s = text;

    // 1. Currencies (e.g. R$ 150,50 -> cento e cinquenta reais e cinquenta centavos)
    static const QRegularExpression realRe(QStringLiteral(R"(R\$\s*(\d+)(?:[.,](\d{1,2}))?)"));
    s = replaceAll(s, realRe, [](const QRegularExpressionMatch &m) {
        const qint64 intPart = m.captured(1).toLongLong();
        const QString centsStr = m.captured(2);
        QString res;
        if (intPart == 1)
            res = QStringLiteral("um real");
        else if (intPart > 0)
            res = numberToPortugueseWords(intPart) + QStringLiteral(" reais");

        if (!centsStr.isEmpty()) {
            int cents = centsStr.toInt();
            if (centsStr.length() == 1)
                cents *= 10;
            if (cents > 0) {
                const QString centsWords = cents == 1
                    ? QStringLiteral("um centavo")
                    : numberToPortugueseWords(cents) + QStringLiteral(" centavos");
                if (res.isEmpty())
                    res = centsWords;
                else
                    res += QStringLiteral(" e ") + centsWords;
            }
        }
        return res.isEmpty() ? QStringLiteral("zero reais") : res;
    });

    // Dollars & Euros ($ 100, € 50)
    static const QRegularExpression dollarRe(QStringLiteral(R"((?:US\$|\$)\s*(\d+))"));
    s = replaceAll(s, dollarRe, [](const QRegularExpressionMatch &m) {
        const qint64 v = m.captured(1).toLongLong();
        return v == 1 ? QStringLiteral("um dólar") : numberToPortugueseWords(v) + QStringLiteral(" dólares");
    });
    static const QRegularExpression euroRe(QStringLiteral(R"(€\s*(\d+))"));
    s = replaceAll(s, euroRe, [](const QRegularExpressionMatch &m) {
        const qint64 v = m.captured(1).toLongLong();
        return v == 1 ? QStringLiteral("um euro") : numberToPortugueseWords(v) + QStringLiteral(" euros");
    });

    // 2. Percentages (50%, 10,5%)
    static const QRegularExpression pctRe(QStringLiteral(R"((\d+)(?:[.,](\d+))?\s*%)"));
    s = replaceAll(s, pctRe, [](const QRegularExpressionMatch &m) {
        const qint64 intPart = m.captured(1).toLongLong();
        const QString decPart = m.captured(2);
        if (!decPart.isEmpty()) {
            return numberToPortugueseWords(intPart) + QStringLiteral(" vírgula ") + numberToPortugueseWords(decPart.toLongLong()) + QStringLiteral(" por cento");
        }
        return numberToPortugueseWords(intPart) + QStringLiteral(" por cento");
    });

    // 3. Time / Hours (14h, 14h30, 14:30)
    static const QRegularExpression timeColonRe(QStringLiteral(R"(\b(\d{1,2}):(\d{2})\b)"));
    s = replaceAll(s, timeColonRe, [](const QRegularExpressionMatch &m) {
        const int h = m.captured(1).toInt();
        const int min = m.captured(2).toInt();
        const QString hStr = h == 1 ? QStringLiteral("uma hora") : numberToPortugueseWords(h) + QStringLiteral(" horas");
        if (min == 0)
            return hStr;
        if (min == 30)
            return hStr + QStringLiteral(" e meia");
        return hStr + QStringLiteral(" e ") + (min == 1 ? QStringLiteral("um minuto") : numberToPortugueseWords(min) + QStringLiteral(" minutos"));
    });

    static const QRegularExpression timeHRe(QStringLiteral(R"(\b(\d{1,2})h(\d{1,2})?\b)"));
    s = replaceAll(s, timeHRe, [](const QRegularExpressionMatch &m) {
        const int h = m.captured(1).toInt();
        const QString minStr = m.captured(2);
        const QString hStr = h == 1 ? QStringLiteral("uma hora") : numberToPortugueseWords(h) + QStringLiteral(" horas");
        if (minStr.isEmpty() || minStr.toInt() == 0)
            return hStr;
        const int min = minStr.toInt();
        if (min == 30)
            return hStr + QStringLiteral(" e meia");
        return hStr + QStringLiteral(" e ") + (min == 1 ? QStringLiteral("um minuto") : numberToPortugueseWords(min) + QStringLiteral(" minutos"));
    });

    // 4. Dates (25/12 or 25/12/2026)
    static const QRegularExpression dateRe(QStringLiteral(R"(\b(\d{1,2})/(\d{1,2})(?:/(\d{2,4}))?\b)"));
    s = replaceAll(s, dateRe, [](const QRegularExpressionMatch &m) {
        const int d = m.captured(1).toInt();
        const int mon = m.captured(2).toInt();
        const QString yStr = m.captured(3);
        if (mon >= 1 && mon <= 12 && d >= 1 && d <= 31) {
            QString out = (d == 1 ? QStringLiteral("primeiro") : numberToPortugueseWords(d))
                          + QStringLiteral(" de ") + kMonthsPt.at(mon);
            if (!yStr.isEmpty()) {
                qint64 y = yStr.toLongLong();
                if (y < 100) y += 2000;
                out += QStringLiteral(" de ") + numberToPortugueseWords(y);
            }
            return out;
        }
        return m.captured(0);
    });

    // 5. Ordinals (1º, 2ª, 10º)
    static const QRegularExpression ordMRe(QStringLiteral(R"(\b(\d+)[º°]\b)"));
    s = replaceAll(s, ordMRe, [](const QRegularExpressionMatch &m) {
        return ordinalToPortuguese(m.captured(1).toInt(), false);
    });
    static const QRegularExpression ordFRe(QStringLiteral(R"(\b(\d+)ª\b)"));
    s = replaceAll(s, ordFRe, [](const QRegularExpressionMatch &m) {
        return ordinalToPortuguese(m.captured(1).toInt(), true);
    });

    // 6. Common units (km, kg, m, cm, min, s)
    static const QRegularExpression kmRe(QStringLiteral(R"(\b(\d+)\s*km\b)"));
    s = replaceAll(s, kmRe, [](const QRegularExpressionMatch &m) {
        const qint64 v = m.captured(1).toLongLong();
        return numberToPortugueseWords(v) + (v == 1 ? QStringLiteral(" quilômetro") : QStringLiteral(" quilômetros"));
    });
    static const QRegularExpression kgRe(QStringLiteral(R"(\b(\d+)\s*kg\b)"));
    s = replaceAll(s, kgRe, [](const QRegularExpressionMatch &m) {
        const qint64 v = m.captured(1).toLongLong();
        return numberToPortugueseWords(v) + (v == 1 ? QStringLiteral(" quilo") : QStringLiteral(" quilos"));
    });
    static const QRegularExpression minRe(QStringLiteral(R"(\b(\d+)\s*min\b)"));
    s = replaceAll(s, minRe, [](const QRegularExpressionMatch &m) {
        const qint64 v = m.captured(1).toLongLong();
        return numberToPortugueseWords(v) + (v == 1 ? QStringLiteral(" minuto") : QStringLiteral(" minutos"));
    });

    // 7. Decimal numbers with comma (e.g. 3,5)
    static const QRegularExpression decRe(QStringLiteral(R"(\b(\d+),(\d+)\b)"));
    s = replaceAll(s, decRe, [](const QRegularExpressionMatch &m) {
        return numberToPortugueseWords(m.captured(1).toLongLong())
               + QStringLiteral(" vírgula ")
               + numberToPortugueseWords(m.captured(2).toLongLong());
    });

    // 8. Standalone integers
    static const QRegularExpression intRe(QStringLiteral(R"(\b\d+\b)"));
    s = replaceAll(s, intRe, [](const QRegularExpressionMatch &m) {
        return numberToPortugueseWords(m.captured(0).toLongLong());
    });

    return s;
}

TtsSynthesizeResult TtsSynthesizer::synthesize(const QString &text,
                                              const QString &voiceId,
                                              double rate,
                                              double pitch)
{
    TtsSynthesizeResult res;
    // Apply automatic Text Normalization (spelling out numbers, currencies, dates, ordinals in Portuguese)
    const QString cleanText = normalizeTextForTts(text).trimmed();
    if (cleanText.isEmpty()) {
        res.error = QStringLiteral("Texto vazio para síntese de voz.");
        return res;
    }

    QDir outDir(ttsOutputDir());
    if (!outDir.exists())
        outDir.mkpath(QStringLiteral("."));

    // Map friendly / legacy IDs to Edge neural short names
    QString targetVoice = voiceId;
    if (targetVoice.isEmpty() || targetVoice.contains(QStringLiteral("Thalita"), Qt::CaseInsensitive)) {
        targetVoice = QStringLiteral("pt-BR-ThalitaMultilingualNeural");
    } else if (targetVoice.contains(QStringLiteral("Antonio"), Qt::CaseInsensitive)) {
        targetVoice = QStringLiteral("pt-BR-AntonioNeural");
    } else if (targetVoice.contains(QStringLiteral("Francisca"), Qt::CaseInsensitive)) {
        targetVoice = QStringLiteral("pt-BR-FranciscaNeural");
    } else if (targetVoice.contains(QStringLiteral("Fabio"), Qt::CaseInsensitive) || targetVoice.contains(QStringLiteral("Andrew"), Qt::CaseInsensitive)) {
        targetVoice = QStringLiteral("en-US-AndrewMultilingualNeural");
    } else if (targetVoice.contains(QStringLiteral("Yara"), Qt::CaseInsensitive) || targetVoice.contains(QStringLiteral("Ava"), Qt::CaseInsensitive)) {
        targetVoice = QStringLiteral("en-US-AvaMultilingualNeural");
    }

    const QString uniqueId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    const QString outFilePath = outDir.filePath(QStringLiteral("tts_%1.mp3").arg(uniqueId));

#if defined(Q_OS_WIN)
    const int rateInt = static_cast<int>(std::round((rate - 1.0) * 100.0));
    const QString rateStr = rateInt >= 0 ? QStringLiteral("+%1%").arg(rateInt) : QStringLiteral("%1%").arg(rateInt);

    const int pitchInt = static_cast<int>(std::round((pitch - 1.0) * 100.0));
    const QString pitchStr = pitchInt >= 0 ? QStringLiteral("+%1%").arg(pitchInt) : QStringLiteral("%1%").arg(pitchInt);

    // Escape text for XML SSML
    QString ssmlText = cleanText;
    ssmlText.replace(QLatin1Char('&'), QStringLiteral("&amp;"));
    ssmlText.replace(QLatin1Char('<'), QStringLiteral("&lt;"));
    ssmlText.replace(QLatin1Char('>'), QStringLiteral("&gt;"));
    ssmlText.replace(QLatin1Char('\''), QStringLiteral("&apos;"));
    ssmlText.replace(QLatin1Char('"'), QStringLiteral("&quot;"));

    // Micro-pauses for punctuation to ensure dynamic human flow
    ssmlText.replace(QRegularExpression(QStringLiteral(R"(,\s*)")), QStringLiteral(", <break time='120ms'/> "));
    ssmlText.replace(QRegularExpression(QStringLiteral(R"(([.!?])\s*)")), QStringLiteral(R"(\1 <break time='240ms'/> )"));

    QString psScript = QStringLiteral(
        "$ErrorActionPreference = 'SilentlyContinue'; "
        "Add-Type -AssemblyName System.Net.Http; "
        "Add-Type -AssemblyName System.Security; "
        "$flags = [System.Reflection.BindingFlags]::NonPublic -bor [System.Reflection.BindingFlags]::Static; "
        "$hInfoField = [System.Net.WebHeaderCollection].GetField('HInfo', $flags); "
        "if ($hInfoField) { "
        "    $hInfo = $hInfoField.GetValue($null); "
        "    $itemProp = $hInfo.GetType().GetProperty('Item', [System.Reflection.BindingFlags]::NonPublic -bor [System.Reflection.BindingFlags]::Instance -bor [System.Reflection.BindingFlags]::Public); "
        "    if ($itemProp) { "
        "        $entry = $itemProp.GetValue($hInfo, @('User-Agent')); "
        "        if ($entry) { "
        "            $isReqField = $entry.GetType().GetField('IsRequestRestricted', [System.Reflection.BindingFlags]::NonPublic -bor [System.Reflection.BindingFlags]::Instance); "
        "            if ($isReqField) { $isReqField.SetValue($entry, $false); } "
        "        } "
        "    } "
        "} "
        "$trustedToken = '6A5AA1D4EAFF4E9FB37E23D68491D6F4'; "
        "$unixSec = [DateTimeOffset]::UtcNow.ToUnixTimeSeconds(); "
        "$winSec = $unixSec + 11644473600; "
        "$roundedSec = $winSec - ($winSec % 300); "
        "$ticks = [double]$roundedSec * 1e7; "
        "$strToHash = ('{0:0}' -f $ticks) + $trustedToken; "
        "$sha256 = [System.Security.Cryptography.SHA256]::Create(); "
        "$hashBytes = $sha256.ComputeHash([System.Text.Encoding]::ASCII.GetBytes($strToHash)); "
        "$secMsGec = -join ($hashBytes | ForEach-Object { '{0:X2}' -f $_ }); "
        "$connId = [Guid]::NewGuid().ToString('N'); "
        "$wsUrl = 'wss://speech.platform.bing.com/consumer/speech/synthesize/readaloud/edge/v1?TrustedClientToken=' + $trustedToken + '&ConnectionId=' + $connId + '&Sec-MS-GEC=' + $secMsGec + '&Sec-MS-GEC-Version=1-143.0.3650.75'; "
        "$ws = New-Object System.Net.WebSockets.ClientWebSocket; "
        "$ws.Options.SetRequestHeader('User-Agent', 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/143.0.0.0 Safari/537.36 Edg/143.0.0.0'); "
        "$ws.Options.SetRequestHeader('Origin', 'chrome-extension://jdiccldimpdaibmpdkjnbmckianbfold'); "
        "$ws.Options.SetRequestHeader('Pragma', 'no-cache'); "
        "$ws.Options.SetRequestHeader('Cache-Control', 'no-cache'); "
        "$muid = [Guid]::NewGuid().ToString('N').ToUpper(); "
        "$ws.Options.SetRequestHeader('Cookie', 'muid=' + $muid + ';'); "
        "$cts = New-Object System.Threading.CancellationTokenSource(15000); "
        "try { $ws.ConnectAsync([Uri]$wsUrl, $cts.Token).Wait(); } catch { exit 1; } "
        "$timestamp = [DateTime]::UtcNow.ToString('yyyy-MM-ddTHH:mm:ss.fffZ'); "
        "$configMsg = 'X-Timestamp:' + $timestamp + \"`r`nContent-Type:application/json; charset=utf-8`r`nPath:speech.config`r`n`r`n{\\\"context\\\":{\\\"synthesis\\\":{\\\"audio\\\":{\\\"metadataoptions\\\":{\\\"sentenceBoundaryEnabled\\\":\\\"false\\\",\\\"wordBoundaryEnabled\\\":\\\"true\\\"},\\\"outputFormat\\\":\\\"audio-24khz-48kbitrate-mono-mp3\\\"}}}}\"; "
        "$configBytes = [System.Text.Encoding]::UTF8.GetBytes($configMsg); "
        "$ws.SendAsync((New-Object ArraySegment[byte] -ArgumentList @(,$configBytes)), [System.Net.WebSockets.WebSocketMessageType]::Text, $true, $cts.Token).Wait(); "
        "$reqId = [Guid]::NewGuid().ToString('N'); "
        "$ssml = \"<speak version='1.0' xmlns='http://www.w3.org/2001/10/synthesis' xml:lang='pt-BR'><voice name='%1'><prosody pitch='%2' rate='%3' volume='+0%'>%4</prosody></voice></speak>\"; "
        "$ssmlMsg = 'X-RequestId:' + $reqId + \"`r`nContent-Type:application/ssml+xml`r`nX-Timestamp:\" + $timestamp + \"`r`nPath:ssml`r`n`r`n\" + $ssml; "
        "$ssmlBytes = [System.Text.Encoding]::UTF8.GetBytes($ssmlMsg); "
        "$ws.SendAsync((New-Object ArraySegment[byte] -ArgumentList @(,$ssmlBytes)), [System.Net.WebSockets.WebSocketMessageType]::Text, $true, $cts.Token).Wait(); "
        "$outFile = '%5'; "
        "$fs = [System.IO.File]::Create($outFile); "
        "$recvBuffer = New-Object byte[] 65536; "
        "while ($ws.State -eq [System.Net.WebSockets.WebSocketState]::Open) { "
        "    $seg = New-Object ArraySegment[byte] -ArgumentList @(,$recvBuffer); "
        "    $res = $ws.ReceiveAsync($seg, $cts.Token).Result; "
        "    if ($res.MessageType -eq [System.Net.WebSockets.WebSocketMessageType]::Close) { break; } "
        "    $cnt = $res.Count; "
        "    if ($cnt -gt 2) { "
        "        if ($res.MessageType -eq [System.Net.WebSockets.WebSocketMessageType]::Binary) { "
        "            $hdrLen = ($recvBuffer[0] -shl 8) -bor $recvBuffer[1]; "
        "            if ($cnt -gt (2 + $hdrLen)) { "
        "                $audioLen = $cnt - (2 + $hdrLen); "
        "                $fs.Write($recvBuffer, 2 + $hdrLen, $audioLen); "
        "            } "
        "        } else { "
        "            $txtMsg = [System.Text.Encoding]::UTF8.GetString($recvBuffer, 0, $cnt); "
        "            if ($txtMsg.Contains('Path:turn.end')) { break; } "
        "        } "
        "    } "
        "} "
        "$fs.Close(); "
        "try { $ws.Dispose(); } catch {}"
    ).arg(targetVoice)
     .arg(pitchStr)
     .arg(rateStr)
     .arg(ssmlText.replace(QLatin1Char('"'), QStringLiteral("`\"")))
     .arg(QDir::toNativeSeparators(outFilePath));

    QProcess proc;
    QStringList args;
    args << QStringLiteral("-NoProfile")
         << QStringLiteral("-NonInteractive")
         << QStringLiteral("-Command")
         << psScript;

    proc.start(QStringLiteral("powershell.exe"), args);
    const bool finished = proc.waitForFinished(20000);

    QFileInfo outFi(outFilePath);
    if (!finished || !outFi.exists() || outFi.size() < 512) {
        // Fallback: Local Windows SAPI synthesiser if network request failed
        const QString fallbackWav = outDir.filePath(QStringLiteral("tts_fallback_%1.wav").arg(uniqueId));
        const bool isFemale = targetVoice.contains(QStringLiteral("Thalita"), Qt::CaseInsensitive)
                           || targetVoice.contains(QStringLiteral("Francisca"), Qt::CaseInsensitive)
                           || targetVoice.contains(QStringLiteral("Ava"), Qt::CaseInsensitive);
        QString fallbackPs = QStringLiteral(
            "$ErrorActionPreference = 'SilentlyContinue'; "
            "Add-Type -AssemblyName System.Speech; "
            "$s = New-Object System.Speech.Synthesis.SpeechSynthesizer; "
            "$g = if (%1) { [System.Speech.Synthesis.VoiceGender]::Female } else { [System.Speech.Synthesis.VoiceGender]::Male }; "
            "try { $s.SelectVoiceByHints([System.Speech.Synthesis.VoiceAge]::Adult, $g, 0, [System.Globalization.CultureInfo]::GetCultureInfo('pt-BR')); } catch {} "
            "$s.SetOutputToWaveFile('%2'); "
            "$s.Speak('%3'); "
            "$s.Dispose();"
        ).arg(isFemale ? QStringLiteral("$true") : QStringLiteral("$false"))
         .arg(QDir::toNativeSeparators(fallbackWav))
         .arg(cleanText.replace(QLatin1Char('\''), QStringLiteral("''")));

        QProcess fallbackProc;
        fallbackProc.start(QStringLiteral("powershell.exe"), {QStringLiteral("-NoProfile"), QStringLiteral("-NonInteractive"), QStringLiteral("-Command"), fallbackPs});
        if (fallbackProc.waitForFinished(10000) && QFile::exists(fallbackWav)) {
            res.ok = true;
            res.filePath = fallbackWav;
            res.durationSeconds = measureWavDuration(fallbackWav);
            if (res.durationSeconds <= 0.05)
                res.durationSeconds = std::max<double>(1.0, cleanText.length() * 0.06 / rate);
            res.cues = generateCuesForText(cleanText, res.durationSeconds);
            return res;
        }

        res.error = QStringLiteral("Falha na síntese de voz neural e offline indisponível.");
        return res;
    }
#elif defined(Q_OS_MACOS)
    // macOS 'say'
    QProcess proc;
    QStringList args;
    if (!voiceId.isEmpty()) {
        args << QStringLiteral("-v") << voiceId;
    }
    args << QStringLiteral("-o") << outFilePath
         << QStringLiteral("--data-format=LEF32@44100")
         << cleanText;
    proc.start(QStringLiteral("say"), args);
    if (!proc.waitForFinished(20000) || !QFile::exists(outFilePath)) {
        res.error = QStringLiteral("Falha na síntese de voz no macOS.");
        return res;
    }
#else
    // Linux espeak-ng fallback
    QProcess proc;
    QStringList args;
    args << QStringLiteral("-w") << outFilePath << cleanText;
    proc.start(QStringLiteral("espeak-ng"), args);
    if (!proc.waitForFinished(20000) || !QFile::exists(outFilePath)) {
        res.error = QStringLiteral("Falha na síntese com espeak-ng.");
        return res;
    }
#endif

    double dur = measureWavDuration(outFilePath);
    if (dur <= 0.05) {
        QFileInfo fi(outFilePath);
        if (fi.suffix().compare(QStringLiteral("mp3"), Qt::CaseInsensitive) == 0 && fi.size() > 0) {
            // CBR 48kbps mono MP3 stream (6000 bytes/sec)
            dur = static_cast<double>(fi.size()) / 6000.0;
        } else {
            dur = std::max<double>(1.0, cleanText.length() * 0.06 / rate);
        }
    }
    res.durationSeconds = dur;

    res.ok = true;
    res.filePath = outFilePath;
    res.cues = generateCuesForText(cleanText, res.durationSeconds);
    return res;
}

} // namespace drift
