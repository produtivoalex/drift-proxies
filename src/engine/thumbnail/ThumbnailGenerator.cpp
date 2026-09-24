// src/engine/thumbnail/ThumbnailGenerator.cpp
// Fase 6B — Gerador de Thumbnails com IA (implementação)

#include "ThumbnailGenerator.h"
#include "engine/MediaThumbnail.h"

#include <QDir>
#include <QPainterPath>
#include <QRadialGradient>
#include <QStandardPaths>
#include <QtConcurrent/QtConcurrent>
#include <cmath>

namespace drift {

ThumbnailGenerator::ThumbnailGenerator(QObject *parent)
    : QObject(parent)
{
}

QList<ThumbnailStyle> ThumbnailGenerator::defaultStyles(const QString &title, const QString &subtitle)
{
    const QString cleanTitle = title.isEmpty() ? QStringLiteral("O SEGREDO REVELADO") : title.toUpper();
    const QString cleanSub = subtitle.isEmpty() ? QStringLiteral("Você não vai acreditar nisso...") : subtitle;

    return {
        // Variante 1: Dark Mystery (Preto, Vermelho & Branco)
        {
            QStringLiteral("dark_mystery"),
            QStringLiteral("Dark Mystery"),
            cleanTitle,
            cleanSub,
            QColor(255, 255, 255),
            QColor(239, 68, 68),
            QColor(0, 0, 0),
            /*useVignette=*/true,
            /*useGlow=*/true,
            QStringLiteral("CHOCANTE!")
        },
        // Variante 2: Viral Gold (Dourado, Preto & Âmbar)
        {
            QStringLiteral("viral_gold"),
            QStringLiteral("Viral Gold"),
            cleanTitle,
            cleanSub,
            QColor(251, 191, 36),
            QColor(245, 158, 11),
            QColor(0, 0, 0),
            /*useVignette=*/true,
            /*useGlow=*/true,
            QStringLiteral("REVELADO!")
        },
        // Variante 3: High Impact (Ciano, Magenta & Contraste Extremo)
        {
            QStringLiteral("high_impact"),
            QStringLiteral("Impacto Máximo"),
            cleanTitle,
            cleanSub,
            QColor(56, 189, 248),
            QColor(236, 72, 153),
            QColor(0, 0, 0),
            /*useVignette=*/true,
            /*useGlow=*/true,
            QStringLiteral("EXCLUSIVO!")
        }
    };
}

void ThumbnailGenerator::generateVariants(const QString &videoPath,
                                          const QString &title,
                                          const QString &subtitle,
                                          bool isPortrait)
{
    emit progressChanged(0.1, tr("Localizando momentos de maior impacto..."));

    // Executar em segundo plano
    QtConcurrent::run([this, videoPath, title, subtitle, isPortrait]() {
        // Criar diretório de saída
        const QString outDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                               QStringLiteral("/thumbnails");
        QDir().mkpath(outDir);

        // Extrair frames-chave em tempos estratégicos
        QList<double> timestamps = { 1.5, 3.0, 5.5, 8.0 };
        QImage bestFrame;
        double bestScore = -1.0;

        for (double t : timestamps) {
            QString thumbPath = MediaThumbnail::generateAtTime(videoPath, t);
            if (!thumbPath.isEmpty() && QFile::exists(thumbPath)) {
                QImage img(thumbPath);
                if (!img.isNull()) {
                    bool hasFace = false;
                    double score = evaluateFrameScore(img, hasFace);
                    if (score > bestScore) {
                        bestScore = score;
                        bestFrame = img;
                    }
                }
            }
        }

        // Se nenhum frame pôde ser extraído pelo decoder, cria uma base estilizada
        if (bestFrame.isNull()) {
            const int w = isPortrait ? 1080 : 1280;
            const int h = isPortrait ? 1920 : 720;
            bestFrame = QImage(w, h, QImage::Format_ARGB32);
            bestFrame.fill(QColor(15, 17, 26));
        }

        QMetaObject::invokeMethod(this, [this]() {
            emit progressChanged(0.5, tr("Renderizando variantes de alta conversão..."));
        });

        const auto styles = defaultStyles(title, subtitle);
        QStringList paths;

        for (int i = 0; i < styles.size(); ++i) {
            const QString outPath = outDir + QStringLiteral("/thumb_variant_%1.jpg").arg(i + 1);
            const QString generated = renderThumbnail(bestFrame, styles[i], outPath, isPortrait);
            if (!generated.isEmpty()) {
                paths.append(generated);
                QMetaObject::invokeMethod(this, [this, i, generated, styles]() {
                    emit variantReady(i, generated, styles[i].templateId);
                });
            }
        }

        QMetaObject::invokeMethod(this, [this, paths]() {
            emit progressChanged(1.0, tr("Thumbnails prontas!"));
            emit allVariantsReady(paths);
        });
    });
}

double ThumbnailGenerator::evaluateFrameScore(const QImage &img, bool &outHasFace)
{
    outHasFace = false;
    if (img.isNull()) return 0.0;

    // Amostragem de luminância e contraste
    int step = 16;
    double sumLuma = 0.0;
    int samples = 0;

    for (int y = 0; y < img.height(); y += step) {
        for (int x = 0; x < img.width(); x += step) {
            QRgb rgb = img.pixel(x, y);
            double luma = 0.299 * qRed(rgb) + 0.587 * qGreen(rgb) + 0.114 * qBlue(rgb);
            sumLuma += luma;
            ++samples;
        }
    }

    if (samples == 0) return 0.0;
    double meanLuma = sumLuma / samples;

    // Penaliza imagens muito escuras (< 25) ou estouradas (> 230)
    double score = 1.0;
    if (meanLuma < 25.0) score *= 0.3;
    else if (meanLuma > 230.0) score *= 0.5;
    else score = (meanLuma / 128.0);

    return score;
}

QString ThumbnailGenerator::renderThumbnail(const QImage &baseFrame,
                                            const ThumbnailStyle &style,
                                            const QString &outputPath,
                                            bool isPortrait)
{
    const int targetW = isPortrait ? 1080 : 1280;
    const int targetH = isPortrait ? 1920 : 720;

    // Escala imagem mantendo proporção e recortando excessos
    QImage canvas = baseFrame.scaled(targetW, targetH, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    if (canvas.width() > targetW || canvas.height() > targetH) {
        int cx = (canvas.width() - targetW) / 2;
        int cy = (canvas.height() - targetH) / 2;
        canvas = canvas.copy(cx, cy, targetW, targetH);
    }

    QPainter p(&canvas);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);

    // 1. Vinheta / Gradiente Dark
    if (style.useVignette) {
        applyVignette(p, targetW, targetH, 0.75);
    }

    // Gradiente inferior escuro para legibilidade perfeita do texto
    QLinearGradient textBg(0, targetH * 0.45, 0, targetH);
    textBg.setColorAt(0.0, QColor(0, 0, 0, 0));
    textBg.setColorAt(0.4, QColor(0, 0, 0, 160));
    textBg.setColorAt(1.0, QColor(0, 0, 0, 240));
    p.fillRect(0, targetH * 0.45, targetW, targetH * 0.55, textBg);

    // 2. Badge Viral no canto superior
    if (!style.badgeText.isEmpty()) {
        const int bx = isPortrait ? 40 : 50;
        const int by = isPortrait ? 80 : 45;
        drawBadge(p, bx, by, style.badgeText, style.accentColor, QColor(255, 255, 255));
    }

    // 3. Título Principal em Tipografia Pesada (Impact/Inter Black)
    QFont titleFont(QStringLiteral("Impact"));
    titleFont.setPixelSize(isPortrait ? 82 : 72);
    titleFont.setBold(true);

    const int textMargin = isPortrait ? 40 : 50;
    const int textY = isPortrait ? (targetH - 380) : (targetH - 240);
    const QRect titleRect(textMargin, textY, targetW - (textMargin * 2), isPortrait ? 220 : 140);

    drawGlowText(p, titleRect, style.title, titleFont, style.primaryColor,
                 style.accentColor, style.strokeColor, /*strokeWidth=*/8);

    // 4. Subtítulo com Pill de Destaque
    if (!style.subtitle.isEmpty()) {
        QFont subFont(QStringLiteral("Arial"));
        subFont.setPixelSize(isPortrait ? 36 : 28);
        subFont.setWeight(QFont::DemiBold);

        const int subY = isPortrait ? (targetH - 140) : (targetH - 85);
        QFontMetrics fm(subFont);
        const int subW = fm.horizontalAdvance(style.subtitle) + 32;
        const int subH = fm.height() + 16;
        const QRect subPill(textMargin, subY, subW, subH);

        // Pill background
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0, 0, 0, 200));
        p.drawRoundedRect(subPill, 8, 8);

        // Subtitle text
        p.setPen(QColor(226, 232, 240));
        p.setFont(subFont);
        p.drawText(subPill, Qt::AlignCenter, style.subtitle);
    }

    p.end();

    if (canvas.save(outputPath, "JPG", 95)) {
        return outputPath;
    }
    return QString();
}

void ThumbnailGenerator::applyVignette(QPainter &p, int width, int height, double intensity)
{
    QRadialGradient grad(width / 2.0, height / 2.0, std::max(width, height) * 0.7);
    grad.setColorAt(0.0, QColor(0, 0, 0, 0));
    grad.setColorAt(0.65, QColor(0, 0, 0, static_cast<int>(80 * intensity)));
    grad.setColorAt(1.0, QColor(0, 0, 0, static_cast<int>(255 * intensity)));
    p.fillRect(0, 0, width, height, grad);
}

void ThumbnailGenerator::drawGlowText(QPainter &p, const QRect &rect, const QString &text,
                                      const QFont &font, const QColor &textColor,
                                      const QColor &glowColor, const QColor &strokeColor,
                                      int strokeWidth)
{
    p.setFont(font);

    QPainterPath path;
    path.addText(rect.x(), rect.y() + font.pixelSize(), font, text);

    // Glow suave
    QPen glowPen(glowColor, strokeWidth + 6);
    glowPen.setJoinStyle(Qt::RoundJoin);
    p.setPen(glowPen);
    p.setBrush(Qt::NoBrush);
    p.drawPath(path);

    // Contorno escuro sólido (outline)
    QPen strokePen(strokeColor, strokeWidth);
    strokePen.setJoinStyle(Qt::RoundJoin);
    p.setPen(strokePen);
    p.drawPath(path);

    // Preenchimento de texto
    p.setPen(Qt::NoPen);
    p.setBrush(textColor);
    p.drawPath(path);
}

void ThumbnailGenerator::drawBadge(QPainter &p, int x, int y, const QString &text,
                                   const QColor &bgColor, const QColor &textColor)
{
    QFont badgeFont(QStringLiteral("Arial"));
    badgeFont.setPixelSize(14);
    badgeFont.setBold(true);

    QFontMetrics fm(badgeFont);
    const int padX = 14;
    const int padY = 8;
    const int w = fm.horizontalAdvance(text) + (padX * 2);
    const int h = fm.height() + (padY * 2);

    const QRect rect(x, y, w, h);

    // Sombra do badge
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 160));
    p.drawRoundedRect(rect.translated(2, 2), 6, 6);

    // Fundo do badge
    p.setBrush(bgColor);
    p.drawRoundedRect(rect, 6, 6);

    // Texto
    p.setPen(textColor);
    p.setFont(badgeFont);
    p.drawText(rect, Qt::AlignCenter, text);
}

} // namespace drift
