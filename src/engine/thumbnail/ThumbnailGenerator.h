// src/engine/thumbnail/ThumbnailGenerator.h
// Fase 6B — Gerador de Thumbnails com IA (Dark Studio)
// Gera miniaturas de alta conversão analisando os melhores frames do vídeo e aplicando estilos virais.
#pragma once

#include <QColor>
#include <QFont>
#include <QImage>
#include <QList>
#include <QObject>
#include <QPainter>
#include <QString>
#include <QStringList>

namespace drift {

struct ThumbnailStyle {
    QString templateId;     // "dark_mystery", "viral_gold", "high_impact"
    QString name;           // "Dark Mystery", "Viral Gold", "Impacto Máximo"
    QString title;          // Texto principal
    QString subtitle;       // Texto secundário
    QColor primaryColor;    // Cor do título
    QColor accentColor;     // Cor do brilho/destaque
    QColor strokeColor;     // Contorno da fonte
    bool useVignette = true;
    bool useGlow = true;
    QString badgeText;      // Tag badge ("SEGREDO", "NOVO", "VIRAL")
};

struct ThumbnailCandidate {
    QString imagePath;
    QImage image;
    double timestampSec = 0.0;
    double score = 0.0;
    bool hasFace = false;
};

class ThumbnailGenerator : public QObject {
    Q_OBJECT

public:
    explicit ThumbnailGenerator(QObject *parent = nullptr);
    ~ThumbnailGenerator() override = default;

    // Gera automaticamente 3 variantes de thumbnail de alta conversão
    Q_INVOKABLE void generateVariants(const QString &videoPath,
                                      const QString &title,
                                      const QString &subtitle = QString(),
                                      bool isPortrait = false);

    // Renderiza uma thumbnail personalizada com estilo
    QString renderThumbnail(const QImage &baseFrame,
                            const ThumbnailStyle &style,
                            const QString &outputPath,
                            bool isPortrait = false);

    // Retorna estilos pré-definidos do Dark Studio
    static QList<ThumbnailStyle> defaultStyles(const QString &title, const QString &subtitle);

signals:
    void progressChanged(double fraction, const QString &status);
    void variantReady(int variantIndex, const QString &imagePath, const QString &templateId);
    void allVariantsReady(const QStringList &paths);

private:
    double evaluateFrameScore(const QImage &img, bool &outHasFace);
    void applyVignette(QPainter &p, int width, int height, double intensity = 0.6);
    void drawGlowText(QPainter &p, const QRect &rect, const QString &text,
                      const QFont &font, const QColor &textColor,
                      const QColor &glowColor, const QColor &strokeColor,
                      int strokeWidth);
    void drawBadge(QPainter &p, int x, int y, const QString &text,
                   const QColor &bgColor, const QColor &textColor);
};

} // namespace drift
