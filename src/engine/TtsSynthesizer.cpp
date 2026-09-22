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

#if defined(Q_OS_WIN)
    // Query Windows installed voices using lightweight powershell query
    QProcess proc;
    QStringList args;
    args << QStringLiteral("-NoProfile")
         << QStringLiteral("-NonInteractive")
         << QStringLiteral("-Command")
         << QStringLiteral(
             "Add-Type -AssemblyName System.Speech; "
             "$s = New-Object System.Speech.Synthesis.SpeechSynthesizer; "
             "foreach ($v in $s.GetInstalledVoices()) { "
             "  Write-Output ($v.VoiceInfo.Name + '|' + $v.VoiceInfo.Culture.Name + '|' + $v.VoiceInfo.Gender); "
             "}"
         );

    proc.start(QStringLiteral("powershell.exe"), args);
    if (proc.waitForFinished(3000)) {
        const QString out = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
        const QStringList lines = out.split(QRegularExpression(QStringLiteral("[\r\n]+")), Qt::SkipEmptyParts);
        for (const QString &line : lines) {
            const QStringList parts = line.split(QLatin1Char('|'));
            if (parts.size() >= 2) {
                TtsVoiceInfo info;
                info.id = parts[0].trimmed();
                info.name = info.id;
                info.lang = parts[1].trimmed();
                if (parts.size() >= 3) {
                    info.gender = parts[2].trimmed();
                }

                // Friendly display name
                if (info.id.contains(QStringLiteral("Maria"), Qt::CaseInsensitive)) {
                    info.name = QStringLiteral("Maria (Português - Brasil)");
                    info.gender = QStringLiteral("Feminino");
                } else if (info.id.contains(QStringLiteral("Daniel"), Qt::CaseInsensitive)) {
                    info.name = QStringLiteral("Daniel (Português - Brasil)");
                    info.gender = QStringLiteral("Masculino");
                } else if (info.id.contains(QStringLiteral("Francisca"), Qt::CaseInsensitive)) {
                    info.name = QStringLiteral("Francisca (Português - Brasil Neural)");
                    info.gender = QStringLiteral("Feminino");
                    info.isNeural = true;
                } else if (info.id.contains(QStringLiteral("Antonio"), Qt::CaseInsensitive)) {
                    info.name = QStringLiteral("Antonio (Português - Brasil Neural)");
                    info.gender = QStringLiteral("Masculino");
                    info.isNeural = true;
                } else if (info.id.contains(QStringLiteral("Zira"), Qt::CaseInsensitive)) {
                    info.name = QStringLiteral("Zira (English - US)");
                    info.gender = QStringLiteral("Feminino");
                } else if (info.id.contains(QStringLiteral("David"), Qt::CaseInsensitive)) {
                    info.name = QStringLiteral("David (English - US)");
                    info.gender = QStringLiteral("Masculino");
                }

                voices.append(info);
            }
        }
    }

    // Always ensure at least primary default Portuguese and English voices exist as options
    if (voices.isEmpty()) {
        TtsVoiceInfo v1;
        v1.id = QStringLiteral("Microsoft Maria Desktop");
        v1.name = QStringLiteral("Maria (Português - Brasil)");
        v1.lang = QStringLiteral("pt-BR");
        v1.gender = QStringLiteral("Feminino");
        voices.append(v1);

        TtsVoiceInfo v2;
        v2.id = QStringLiteral("Microsoft Daniel");
        v2.name = QStringLiteral("Daniel (Português - Brasil)");
        v2.lang = QStringLiteral("pt-BR");
        v2.gender = QStringLiteral("Masculino");
        voices.append(v2);

        TtsVoiceInfo v3;
        v3.id = QStringLiteral("Microsoft Zira Desktop");
        v3.name = QStringLiteral("Zira (English - US)");
        v3.lang = QStringLiteral("en-US");
        v3.gender = QStringLiteral("Feminino");
        voices.append(v3);
    }
#elif defined(Q_OS_MACOS)
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
                voices.append(info);
            }
        }
    }
#else
    // Linux / other: check espeak or piper
    TtsVoiceInfo v;
    v.id = QStringLiteral("pt-br");
    v.name = QStringLiteral("Voz em Português (espeak-ng)");
    v.lang = QStringLiteral("pt-BR");
    v.gender = QStringLiteral("Neutro");
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
            nv.name = f.section(QLatin1Char('.'), 0, 0) + QStringLiteral(" (Neural Offline)");
            nv.lang = f.startsWith(QStringLiteral("pt_BR"), Qt::CaseInsensitive) ? QStringLiteral("pt-BR") : QStringLiteral("en-US");
            nv.gender = QStringLiteral("Neural");
            nv.isNeural = true;
            voices.prepend(nv);
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

TtsSynthesizeResult TtsSynthesizer::synthesize(const QString &text,
                                              const QString &voiceId,
                                              double rate,
                                              double pitch)
{
    Q_UNUSED(pitch);
    TtsSynthesizeResult res;
    const QString cleanText = text.trimmed();
    if (cleanText.isEmpty()) {
        res.error = QStringLiteral("Texto vazio para síntese de voz.");
        return res;
    }

    QDir outDir(ttsOutputDir());
    if (!outDir.exists())
        outDir.mkpath(QStringLiteral("."));

    const QString uniqueId = QUuid::createUuid().toString(QUuid::WithoutBraces).left(8);
    const QString outFilePath = outDir.filePath(QStringLiteral("tts_%1.wav").arg(uniqueId));

#if defined(Q_OS_WIN)
    // Convert rate (0.5 to 2.0) to PowerShell SpeechSynthesizer Rate (-10 to 10)
    // 1.0 -> 0, 0.5 -> -5, 2.0 -> 5
    int psRate = 0;
    if (rate < 1.0) {
        psRate = static_cast<int>(std::round((rate - 1.0) * 10.0));
    } else {
        psRate = static_cast<int>(std::round((rate - 1.0) * 5.0));
    }
    psRate = qBound(-10, psRate, 10);

    // Escape text for powershell single quotes
    QString escapedText = cleanText;
    escapedText.replace(QLatin1Char('\''), QStringLiteral("''"));
    escapedText.replace(QLatin1Char('\n'), QStringLiteral(" "));
    escapedText.replace(QLatin1Char('\r'), QStringLiteral(" "));

    QString escapedVoice = voiceId;
    escapedVoice.replace(QLatin1Char('\''), QStringLiteral("''"));

    // PowerShell synthesis script
    QString psScript = QStringLiteral(
        "Add-Type -AssemblyName System.Speech; "
        "$s = New-Object System.Speech.Synthesis.SpeechSynthesizer; "
    );

    if (!escapedVoice.isEmpty() && !escapedVoice.endsWith(QStringLiteral(".onnx"))) {
        psScript += QStringLiteral(
            "try { $s.SelectVoice('%1'); } catch {} "
        ).arg(escapedVoice);
    }

    psScript += QStringLiteral(
        "$s.Rate = %1; "
        "$s.SetOutputToWaveFile('%2'); "
        "$s.Speak('%3'); "
        "$s.Dispose();"
    ).arg(psRate).arg(QDir::toNativeSeparators(outFilePath)).arg(escapedText);

    QProcess proc;
    QStringList args;
    args << QStringLiteral("-NoProfile")
         << QStringLiteral("-NonInteractive")
         << QStringLiteral("-Command")
         << psScript;

    proc.start(QStringLiteral("powershell.exe"), args);
    const bool finished = proc.waitForFinished(20000);

    if (!finished || proc.exitCode() != 0 || !QFile::exists(outFilePath)) {
        res.error = QStringLiteral("Falha na síntese de voz nativa: ") + QString::fromUtf8(proc.readAllStandardError());
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

    const double dur = measureWavDuration(outFilePath);
    if (dur <= 0.05) {
        // Fallback estimate if WAV header parsing missed
        const double estimatedSec = std::max<double>(1.0, cleanText.length() * 0.06 / rate);
        res.durationSeconds = estimatedSec;
    } else {
        res.durationSeconds = dur;
    }

    res.ok = true;
    res.filePath = outFilePath;
    res.cues = generateCuesForText(cleanText, res.durationSeconds);
    return res;
}

} // namespace drift
