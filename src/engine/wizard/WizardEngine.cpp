#include "WizardEngine.h"

#include "core/Project.h"
#include "core/TimelineOps.h"
#include "engine/TtsSynthesizer.h"
#include "engine/WhisperTranscriber.h"
#include "engine/ClipReaderPool.h"

#include <QThread>
#include <QMetaObject>
#include <QFileInfo>
#include <QUuid>

namespace drift {

WizardEngine::WizardEngine(QObject *parent) : QObject(parent)
{
}

WizardEngine::~WizardEngine()
{
    cancel();
}

void WizardEngine::cancel()
{
    m_cancel.store(true, std::memory_order_relaxed);
}

void WizardEngine::generateTimeline(const QString &script, const QString &vibe, const QString &voiceId, Project *project)
{
    m_cancel.store(false, std::memory_order_relaxed);

    QThread *thread = QThread::create([this, script, vibe, voiceId, project]() {
        processAsync(script, vibe, voiceId, project);
    });
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start();
}

void WizardEngine::processAsync(QString script, QString vibe, QString voiceId, Project *project)
{
    auto updateProgress = [this](double fraction, const QString &status) {
        QMetaObject::invokeMethod(this, "progressChanged", Qt::QueuedConnection,
                                  Q_ARG(double, fraction), Q_ARG(QString, status));
    };

    auto finish = [this](bool success, const QString &error) {
        QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection,
                                  Q_ARG(bool, success), Q_ARG(QString, error));
    };

    if (m_cancel.load(std::memory_order_relaxed)) {
        finish(false, tr("Cancelado pelo usuário."));
        return;
    }

    updateProgress(0.1, tr("Sintetizando voz do roteiro..."));

    TtsSynthesizer &tts = TtsSynthesizer::instance();
    TtsSynthesizeResult ttsResult = tts.synthesize(script, voiceId);

    if (!ttsResult.ok) {
        finish(false, tr("Falha no TTS: ") + ttsResult.error);
        return;
    }

    if (m_cancel.load(std::memory_order_relaxed)) {
        finish(false, tr("Cancelado pelo usuário."));
        return;
    }

    updateProgress(0.5, tr("Sincronizando fala com a timeline (Whisper)..."));

    WhisperTranscriber &whisper = WhisperTranscriber::instance();
    if (!whisper.available()) {
        finish(false, tr("Whisper indisponível: ") + whisper.lastError());
        return;
    }

    // Lendo o áudio gerado pelo TTS
    const int rate = 16000;
    const int chunkFrames = 30 * rate;
    std::vector<float> mono;
    drift::TimeUs pos = 0;
    const drift::TimeUs totalAudioUs = static_cast<drift::TimeUs>(ttsResult.durationSeconds * drift::kUsPerSecond);

    while (pos < totalAudioUs) {
        if (m_cancel.load(std::memory_order_relaxed)) {
            finish(false, tr("Cancelado pelo usuário."));
            return;
        }

        const drift::TimeUs remainUs = totalAudioUs - pos;
        const int frames = qMin<int64_t>(chunkFrames, (remainUs * rate) / drift::kUsPerSecond + 1);
        
        if (frames <= 0) break;

        QVector<float> stereo(static_cast<qsizetype>(frames) * 2);
        const int got = ClipReaderPool::instance().readAudioInterleaved(
            ttsResult.filePath, qHash(ttsResult.filePath), pos, frames, rate, stereo.data()
        );
        
        if (got <= 0) break;

        const size_t base = mono.size();
        mono.resize(base + got);
        for (int i = 0; i < got; ++i) {
            mono[base + i] = 0.5f * (stereo[i * 2] + stereo[i * 2 + 1]);
        }
        pos += static_cast<drift::TimeUs>((static_cast<int64_t>(got) * drift::kUsPerSecond) / rate);
    }

    if (m_cancel.load(std::memory_order_relaxed)) {
        finish(false, tr("Cancelado pelo usuário."));
        return;
    }

    updateProgress(0.7, tr("Mapeando tempos das falas..."));

    WhisperResult wResult = whisper.transcribe(mono, [this, &updateProgress](double p, const QString &status) {
        if (m_cancel.load(std::memory_order_relaxed)) return false;
        updateProgress(0.7 + (p * 0.2), status);
        return true;
    });

    if (m_cancel.load(std::memory_order_relaxed)) {
        finish(false, tr("Cancelado pelo usuário."));
        return;
    }

    if (!wResult.ok && !wResult.cancelled) {
        finish(false, tr("Falha no alinhamento Whisper: ") + wResult.error);
        return;
    }

    updateProgress(0.95, tr("Montando Dark Studio Timeline..."));

    // Aplicar no projeto
    QMetaObject::invokeMethod(project, [project, ttsResult, wResult]() {
        int audioTrackIndex = drift::ensureTrackForClipType(*project, ClipType::Audio);

        Clip audioClip;
        audioClip.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        audioClip.type = ClipType::Audio;
        audioClip.path = ttsResult.filePath;
        audioClip.start = 0;
        audioClip.duration = static_cast<drift::TimeUs>(ttsResult.durationSeconds * drift::kUsPerSecond);
        audioClip.sourceStart = 0;
        audioClip.name = QFileInfo(ttsResult.filePath).fileName();

        project->tracks()[audioTrackIndex].clips.append(audioClip);

        // Se o resultado do Whisper for bem-sucedido, adicione as legendas
        if (wResult.ok && !wResult.cues.isEmpty()) {
            int subtitleTrackIndex = drift::ensureTrackForClipType(*project, ClipType::Subtitle);
            for (const SubtitleCue &cue : wResult.cues) {
                Clip subClip;
                subClip.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
                subClip.type = ClipType::Subtitle;
                subClip.start = cue.startUs;
                subClip.duration = cue.durationUs;
                subClip.name = cue.text;
                // Outras propriedades como fonte ou cor podem ser definidas posteriormente.
                project->tracks()[subtitleTrackIndex].clips.append(subClip);
            }
        }
    }, Qt::QueuedConnection);

    finish(true, "");
}

} // namespace drift
