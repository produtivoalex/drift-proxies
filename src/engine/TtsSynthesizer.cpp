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
    vThalita.id = QStringLiteral("pt-BR-ThalitaNeural");
    vThalita.name = QStringLiteral("Thalita (🔥 Viral & Espontânea)");
    vThalita.lang = QStringLiteral("pt-BR");
    vThalita.gender = QStringLiteral("Feminino");
    vThalita.vibeTag = QStringLiteral("🔥 Viral & Espontânea");
    vThalita.description = QStringLiteral("Jovem, enérgica e descontraída. A voz nº 1 para TikTok, Reels e vídeos curtos no Brasil. Zero robótica.");
    vThalita.isFeatured = true;
    vThalita.isNeural = true;
    vThalita.defaultRate = 1.1;
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
    vAntonio.defaultPitch = 0.95;
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
    vFabio.id = QStringLiteral("pt-BR-FabioNeural");
    vFabio.name = QStringLiteral("Fabio (⚡ Tech & Dinâmico)");
    vFabio.lang = QStringLiteral("pt-BR");
    vFabio.gender = QStringLiteral("Masculino");
    vFabio.vibeTag = QStringLiteral("⚡ Tech & Dinâmico");
    vFabio.description = QStringLiteral("Voz jovem, ágil e vibrante. Ideal para vídeos de tecnologia, esportes, novidades e alta retenção.");
    vFabio.isFeatured = true;
    vFabio.isNeural = true;
    vFabio.defaultRate = 1.05;
    vFabio.defaultPitch = 1.0;
    voices.append(vFabio);

    // 5. Yara - Descolada, podcaster, lifestyle e tom conversacional
    TtsVoiceInfo vYara;
    vYara.id = QStringLiteral("pt-BR-YaraNeural");
    vYara.name = QStringLiteral("Yara (💬 Autêntica & Lifestyle)");
    vYara.lang = QStringLiteral("pt-BR");
    vYara.gender = QStringLiteral("Feminino");
    vYara.vibeTag = QStringLiteral("💬 Autêntica & Lifestyle");
    vYara.description = QStringLiteral("Tom descolado e moderno, estilo bate-papo e podcaster. Perfeita para vlogs, lifestyle e conselhos.");
    vYara.isFeatured = true;
    vYara.isNeural = true;
    vYara.defaultRate = 1.0;
    vYara.defaultPitch = 1.02;
    voices.append(vYara);

#if defined(Q_OS_WIN)
    // Also discover installed system voices as fallback
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
                const QString vId = parts[0].trimmed();
                // Avoid duplicating the featured ones
                bool alreadyInList = false;
                for (const auto &v : voices) {
                    if (v.id == vId) { alreadyInList = true; break; }
                }
                if (!alreadyInList) {
                    TtsVoiceInfo info;
                    info.id = vId;
                    info.name = vId;
                    info.lang = parts[1].trimmed();
                    if (parts.size() >= 3) {
                        info.gender = parts[2].trimmed();
                    }
                    info.vibeTag = QStringLiteral("Voz do Sistema");
                    info.description = QStringLiteral("Sintetizador local do Windows");
                    voices.append(info);
                }
            }
        }
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

TtsSynthesizeResult TtsSynthesizer::synthesize(const QString &text,
                                              const QString &voiceId,
                                              double rate,
                                              double pitch)
{
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
    int psRate = 0;
    if (rate < 1.0) {
        psRate = static_cast<int>(std::round((rate - 1.0) * 10.0));
    } else {
        psRate = static_cast<int>(std::round((rate - 1.0) * 5.0));
    }
    psRate = qBound(-10, psRate, 10);

    // Escape text for PowerShell single quotes
    QString escapedText = cleanText;
    escapedText.replace(QLatin1Char('\''), QStringLiteral("''"));
    escapedText.replace(QLatin1Char('\n'), QStringLiteral(" "));
    escapedText.replace(QLatin1Char('\r'), QStringLiteral(" "));

    // Humanize text by inserting natural micro-pauses at punctuation for dynamic, non-robotic flow
    // SSML breath pauses: comma -> 120ms, period/question/exclamation -> 260ms
    QString ssmlText = cleanText;
    ssmlText.replace(QLatin1Char('&'), QStringLiteral("&amp;"));
    ssmlText.replace(QLatin1Char('<'), QStringLiteral("&lt;"));
    ssmlText.replace(QLatin1Char('>'), QStringLiteral("&gt;"));
    ssmlText.replace(QLatin1Char('\''), QStringLiteral("&apos;"));
    ssmlText.replace(QLatin1Char('"'), QStringLiteral("&quot;"));

    // Micro-pauses for punctuation
    ssmlText.replace(QRegularExpression(QStringLiteral(R"(,\s*)")), QStringLiteral(", <break time='120ms'/> "));
    ssmlText.replace(QRegularExpression(QStringLiteral(R"(([.!?])\s*)")), QStringLiteral(R"(\1 <break time='260ms'/> )"));

    // Pitch percentage string for SSML (e.g. pitch=1.05 -> "+5%", pitch=0.95 -> "-5%")
    const int pitchPercent = static_cast<int>(std::round((pitch - 1.0) * 100.0));
    QString pitchStr = pitchPercent >= 0 ? QStringLiteral("+%1%").arg(pitchPercent) : QStringLiteral("%1%").arg(pitchPercent);

    // Determine voice gender preference
    const bool isFemaleVoice = voiceId.contains(QStringLiteral("Thalita"), Qt::CaseInsensitive)
                            || voiceId.contains(QStringLiteral("Francisca"), Qt::CaseInsensitive)
                            || voiceId.contains(QStringLiteral("Yara"), Qt::CaseInsensitive)
                            || voiceId.contains(QStringLiteral("Maria"), Qt::CaseInsensitive)
                            || voiceId.contains(QStringLiteral("Zira"), Qt::CaseInsensitive);

    // PowerShell synthesis script supporting OneCore & SAPI with SSML humanization
    QString psScript = QStringLiteral(
        "$ErrorActionPreference = 'SilentlyContinue'; "
        "Add-Type -AssemblyName System.Speech; "
        "$s = New-Object System.Speech.Synthesis.SpeechSynthesizer; "
        "$voiceId = '%1'; "
        "$targetGender = if (%2) { [System.Speech.Synthesis.VoiceGender]::Female } else { [System.Speech.Synthesis.VoiceGender]::Male }; "
        "try { "
        "  if ($voiceId -ne '' -and -not $voiceId.Contains('Neural')) { $s.SelectVoice($voiceId); } "
        "  else { $s.SelectVoiceByHints([System.Speech.Synthesis.VoiceAge]::Adult, $targetGender, 0, [System.Globalization.CultureInfo]::GetCultureInfo('pt-BR')); } "
        "} catch { "
        "  try { $s.SelectVoiceByHints([System.Speech.Synthesis.VoiceAge]::Adult, $targetGender); } catch {} "
        "} "
        "$s.Rate = %3; "
        "$s.SetOutputToWaveFile('%4'); "
        "$ssml = \"<speak version='1.0' xmlns='http://www.w3.org/2001/10/synthesis' xml:lang='pt-BR'><prosody pitch='%5'>%6</prosody></speak>\"; "
        "try { $s.SpeakSsml($ssml); } catch { $s.Speak('%7'); } "
        "$s.Dispose();"
    ).arg(voiceId)
     .arg(isFemaleVoice ? QStringLiteral("$true") : QStringLiteral("$false"))
     .arg(psRate)
     .arg(QDir::toNativeSeparators(outFilePath))
     .arg(pitchStr)
     .arg(ssmlText.replace(QLatin1Char('"'), QStringLiteral("`\"")))
     .arg(escapedText);

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
