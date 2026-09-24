#pragma once

#include <QColor>
#include <QList>
#include <QString>
#include <QVariantMap>

namespace drift {

struct VirtualBackgroundPreset
{
    QString id;
    QString name;
    QString category;    // "studio", "podcast", "tech", "gradients", "dynamic"
    QString badge;       // "4K Estúdio", "Podcast Pro", "Neon HDR", "Dinâmico", etc.
    QString description;
    QString type;        // "gradient", "solid", "image", "dynamic"
    QColor primaryColor;
    QColor secondaryColor;
    QColor accentColor;
    int gradientType = 0; // 0 = Linear, 1 = Radial, 2 = Sweep
    double defaultBlur = 0.0; // Suggested virtual depth of field (0.0 to 1.0)

    QVariantMap toVariantMap() const
    {
        QVariantMap map;
        map[QStringLiteral("id")] = id;
        map[QStringLiteral("name")] = name;
        map[QStringLiteral("category")] = category;
        map[QStringLiteral("badge")] = badge;
        map[QStringLiteral("description")] = description;
        map[QStringLiteral("type")] = type;
        map[QStringLiteral("primaryColor")] = primaryColor.name(QColor::HexArgb);
        map[QStringLiteral("secondaryColor")] = secondaryColor.name(QColor::HexArgb);
        map[QStringLiteral("accentColor")] = accentColor.name(QColor::HexArgb);
        map[QStringLiteral("gradientType")] = gradientType;
        map[QStringLiteral("defaultBlur")] = defaultBlur;
        return map;
    }
};

class VirtualBackgroundCatalog
{
public:
    static const QList<VirtualBackgroundPreset> &presets();
    static const VirtualBackgroundPreset *findPreset(const QString &id);
};

} // namespace drift
