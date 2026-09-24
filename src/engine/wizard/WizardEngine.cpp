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

    // Aplicar no projeto (FASE 3 - Automação Cinematográfica)
    QMetaObject::invokeMethod(project, [project, ttsResult, wResult, vibe]() {
        int videoTrackIndex = drift::ensureTrackForClipType(*project, ClipType::Video);
        int audioTrackIndex = drift::ensureTrackForClipType(*project, ClipType::Audio);
        int subtitleTrackIndex = drift::ensureTrackForClipType(*project, ClipType::Subtitle);
        
        // Track dedicada para SFX e Bed Track
        drift::Track sfxTrack;
        sfxTrack.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        sfxTrack.type = ClipType::Audio;
        sfxTrack.name = QStringLiteral("SFX / Impactos");
        project->tracks().append(sfxTrack);
        int sfxTrackIndex = project->tracks().size() - 1;

        if (!wResult.ok || wResult.cues.isEmpty()) {
            // Fallback sem legendas (apenas adiciona o áudio completo)
            Clip audioClip;
            audioClip.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
            audioClip.type = ClipType::Audio;
            audioClip.path = ttsResult.filePath;
            audioClip.start = 0;
            audioClip.duration = static_cast<drift::TimeUs>(ttsResult.durationSeconds * drift::kUsPerSecond);
            audioClip.sourceStart = 0;
            audioClip.name = QFileInfo(ttsResult.filePath).fileName();
            project->tracks()[audioTrackIndex].clips.append(audioClip);
            return;
        }

        // Agrupar cues em blocos (Jump Cuts semânticos em pausas > 400ms)
        struct SpeechBlock {
            drift::TimeUs startUs;
            drift::TimeUs endUs;
            QString text;
            QList<SubtitleCue> cues;
        };
        QList<SpeechBlock> blocks;
        SpeechBlock currentBlock;
        for (const SubtitleCue &cue : wResult.cues) {
            if (currentBlock.cues.isEmpty()) {
                currentBlock.startUs = cue.startUs;
                currentBlock.endUs = cue.startUs + cue.durationUs;
                currentBlock.text = cue.text;
                currentBlock.cues.append(cue);
            } else {
                if (cue.startUs - currentBlock.endUs > 400000) { // 400ms gap = silence
                    blocks.append(currentBlock);
                    currentBlock.startUs = cue.startUs;
                    currentBlock.endUs = cue.startUs + cue.durationUs;
                    currentBlock.text = cue.text;
                    currentBlock.cues.clear();
                    currentBlock.cues.append(cue);
                } else {
                    currentBlock.endUs = cue.startUs + cue.durationUs;
                    currentBlock.text += " " + cue.text;
                    currentBlock.cues.append(cue);
                }
            }
        }
        if (!currentBlock.cues.isEmpty()) {
            blocks.append(currentBlock);
        }

        drift::TimeUs currentTimelineUs = 0;
        int blockIndex = 0;

        for (const SpeechBlock &block : blocks) {
            drift::TimeUs blockDuration = block.endUs - block.startUs;

            // 1. Áudio (Jump Cut Semântico)
            Clip audioClip;
            audioClip.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
            audioClip.type = ClipType::Audio;
            audioClip.path = ttsResult.filePath;
            audioClip.start = currentTimelineUs;
            audioClip.duration = blockDuration;
            audioClip.sourceStart = block.startUs;
            audioClip.name = QStringLiteral("Voz (Bloco %1)").arg(blockIndex + 1);
            project->tracks()[audioTrackIndex].clips.append(audioClip);

            // 2. Legendas (cues ajustadas para a timeline contínua do bloco)
            for (const SubtitleCue &cue : block.cues) {
                Clip subClip;
                subClip.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
                subClip.type = ClipType::Subtitle;
                subClip.start = currentTimelineUs + (cue.startUs - block.startUs);
                subClip.duration = cue.durationUs;
                subClip.name = cue.text;
                project->tracks()[subtitleTrackIndex].clips.append(subClip);
            }

            // 3. B-Roll Automático & Ken Burns
            Clip videoClip;
            videoClip.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
            videoClip.type = ClipType::Video;
            // Buscador local simulado pelo nicho/vibe
            videoClip.path = QStringLiteral("app://b-roll/%1_%2.mp4").arg(vibe.toLower()).arg((blockIndex % 5) + 1);
            videoClip.start = currentTimelineUs;
            videoClip.duration = blockDuration;
            videoClip.sourceStart = 0;
            videoClip.name = QStringLiteral("B-Roll: %1").arg(vibe);
            
            // Adição automática da Curva de Velocidade Ken Burns
            videoClip.animCombo.kind = drift::ClipAnimKind::KenBurns;
            videoClip.animCombo.durationUs = blockDuration; // Ken Burns ao longo de todo o clipe
            project->tracks()[videoTrackIndex].clips.append(videoClip);

            // 4. Sonorização Inteligente (Impactos SFX e Auto-Ducking ready)
            QString lowerText = block.text.toLower();
            bool hasImpact = lowerText.contains(QStringLiteral("!")) || 
                             lowerText.contains(QStringLiteral("poder")) || 
                             lowerText.contains(QStringLiteral("segredo")) || 
                             lowerText.contains(QStringLiteral("dinheiro"));
            
            if (hasImpact || blockIndex == 0) {
                Clip sfxClip;
                sfxClip.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
                sfxClip.type = ClipType::Audio;
                sfxClip.path = (blockIndex == 0) ? QStringLiteral("sfx://whoosh_deep") : QStringLiteral("sfx://boom_bass");
                sfxClip.start = currentTimelineUs;
                sfxClip.duration = 1000000; // 1s
                sfxClip.sourceStart = 0;
                sfxClip.name = (blockIndex == 0) ? QStringLiteral("Whoosh Deep") : QStringLiteral("Boom Bass");
                project->tracks()[sfxTrackIndex].clips.append(sfxClip);
            }

            currentTimelineUs += blockDuration;
            blockIndex++;
        }

        // Bed Track (Música de Fundo) com Auto-Ducking
        Clip bedTrack;
        bedTrack.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        bedTrack.type = ClipType::Audio;
        bedTrack.path = QStringLiteral("app://music/bgm_%1.mp3").arg(vibe.toLower());
        bedTrack.start = 0;
        bedTrack.duration = currentTimelineUs; // Cobre todo o vídeo
        bedTrack.sourceStart = 0;
        bedTrack.name = QStringLiteral("Trilha Sonora: %1").arg(vibe);
        
        // Efeito de Auto-Ducking
        drift::Effect duckingEffect;
        duckingEffect.id = QStringLiteral("auto_ducking");
        duckingEffect.label = QStringLiteral("Auto-Ducking");
        bedTrack.audioEffects.append(duckingEffect);
        
        project->tracks()[sfxTrackIndex].clips.append(bedTrack);

    }, Qt::QueuedConnection);

    finish(true, "");
}

} // namespace drift
