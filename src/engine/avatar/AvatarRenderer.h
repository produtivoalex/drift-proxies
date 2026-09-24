// src/engine/avatar/AvatarRenderer.h
// Fase 6C — Avatares 2D com Lip-Sync IA (Dark Studio)
// Apresentador virtual animado com sincronização labial automática a partir do áudio.
#pragma once

#include <QColor>
#include <QImage>
#include <QList>
#include <QMap>
#include <QObject>
#include <QPainter>
#include <QString>
#include <vector>

namespace drift {

enum class AvatarMouthShape {
    Closed = 0,
    SlightOpen = 1,
    Open = 2,
    WideOpen = 3,
    Round = 4
};

struct AvatarKeyframe {
    double timestampSec = 0.0;
    AvatarMouthShape mouthShape = AvatarMouthShape::Closed;
    double eyeBlink = 0.0;        // 0.0 = aberto, 1.0 = fechado
    double headTilt = 0.0;        // ângulo em graus (-3.0 a +3.0)
    double expressionIntensity = 1.0;
};

class AvatarRenderer : public QObject {
    Q_OBJECT

public:
    explicit AvatarRenderer(QObject *parent = nullptr);
    ~AvatarRenderer() override = default;

    // Carrega avatar personalizado ou usa o Apresentador Dark nativo
    Q_INVOKABLE bool loadAvatar(const QString &avatarPackPath = QString());

    // Analisa o áudio e gera os keyframes de lip-sync e microexpressões
    Q_INVOKABLE void analyzeAudio(const QString &audioPath);

    // Renderiza um único frame no instante de tempo indicado
    Q_INVOKABLE QImage renderFrame(double timestampSec, int width = 512, int height = 512);

    // Exporta sequência de frames para composição na timeline
    Q_INVOKABLE void exportFrameSequence(const QString &audioPath,
                                         const QString &outputDir,
                                         int width = 512,
                                         int height = 512,
                                         double fps = 30.0);

    Q_INVOKABLE int keyframeCount() const { return m_keyframes.size(); }
    Q_INVOKABLE QString currentAvatarName() const { return m_avatarName; }

signals:
    void analysisProgress(double fraction, const QString &status);
    void analysisComplete(int keyframeCount);
    void exportProgress(double fraction);
    void exportComplete(const QString &outputDir, int frameCount);

private:
    void generateDefaultProceduralSprites();
    AvatarMouthShape detectMouthShape(double energy, double highFreqRatio) const;
    AvatarKeyframe interpolateKeyframe(double timestampSec) const;

    QString m_avatarName = QStringLiteral("Apresentador Dark");
    QMap<AvatarMouthShape, QImage> m_sprites;
    QList<AvatarKeyframe> m_keyframes;
};

} // namespace drift
