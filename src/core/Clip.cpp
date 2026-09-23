#include "Clip.h"

namespace drift {

QString clipTypeToString(ClipType type)
{
    switch (type) {
    case ClipType::Video:
        return QStringLiteral("video");
    case ClipType::Audio:
        return QStringLiteral("audio");
    case ClipType::Image:
        return QStringLiteral("image");
    case ClipType::Text:
        return QStringLiteral("text");
    case ClipType::Subtitle:
        return QStringLiteral("subtitle");
    case ClipType::Shape:
        return QStringLiteral("shape");
    case ClipType::Adjustment:
        return QStringLiteral("adjustment");
    case ClipType::Vector:
        return QStringLiteral("vector");
    case ClipType::Model3d:
        return QStringLiteral("model3d");
    }
    return QStringLiteral("video");
}

ClipType clipTypeFromString(const QString &type)
{
    if (type == QStringLiteral("audio"))
        return ClipType::Audio;
    if (type == QStringLiteral("image"))
        return ClipType::Image;
    if (type == QStringLiteral("text"))
        return ClipType::Text;
    if (type == QStringLiteral("subtitle"))
        return ClipType::Subtitle;
    if (type == QStringLiteral("shape"))
        return ClipType::Shape;
    if (type == QStringLiteral("adjustment"))
        return ClipType::Adjustment;
    if (type == QStringLiteral("vector"))
        return ClipType::Vector;
    if (type == QStringLiteral("model3d"))
        return ClipType::Model3d;
    return ClipType::Video;
}

QString adjustmentKindToString(AdjustmentKind kind)
{
    switch (kind) {
    case AdjustmentKind::VideoEffects:
        return QStringLiteral("videoEffects");
    case AdjustmentKind::AudioEffects:
        return QStringLiteral("audioEffects");
    case AdjustmentKind::Mask:
        return QStringLiteral("mask");
    }
    return QStringLiteral("videoEffects");
}

AdjustmentKind adjustmentKindFromString(const QString &kind)
{
    if (kind == QStringLiteral("audioEffects"))
        return AdjustmentKind::AudioEffects;
    if (kind == QStringLiteral("mask"))
        return AdjustmentKind::Mask;
    return AdjustmentKind::VideoEffects;
}

QString blendModeToString(BlendMode mode)
{
    switch (mode) {
    case BlendMode::Normal:
        return QStringLiteral("normal");
    case BlendMode::Multiply:
        return QStringLiteral("multiply");
    case BlendMode::Screen:
        return QStringLiteral("screen");
    case BlendMode::Overlay:
        return QStringLiteral("overlay");
    case BlendMode::Add:
        return QStringLiteral("add");
    case BlendMode::Darken:
        return QStringLiteral("darken");
    case BlendMode::Lighten:
        return QStringLiteral("lighten");
    case BlendMode::ColorDodge:
        return QStringLiteral("colorDodge");
    case BlendMode::ColorBurn:
        return QStringLiteral("colorBurn");
    case BlendMode::SoftLight:
        return QStringLiteral("softLight");
    case BlendMode::Difference:
        return QStringLiteral("difference");
    }
    return QStringLiteral("normal");
}

BlendMode blendModeFromString(const QString &mode)
{
    const QString lower = mode.toLower();
    if (lower == QStringLiteral("multiply"))
        return BlendMode::Multiply;
    if (lower == QStringLiteral("screen"))
        return BlendMode::Screen;
    if (lower == QStringLiteral("overlay"))
        return BlendMode::Overlay;
    if (lower == QStringLiteral("add") || lower == QStringLiteral("plus"))
        return BlendMode::Add;
    if (lower == QStringLiteral("darken"))
        return BlendMode::Darken;
    if (lower == QStringLiteral("lighten"))
        return BlendMode::Lighten;
    if (lower == QStringLiteral("colordodge") || lower == QStringLiteral("dodge"))
        return BlendMode::ColorDodge;
    if (lower == QStringLiteral("colorburn") || lower == QStringLiteral("burn"))
        return BlendMode::ColorBurn;
    if (lower == QStringLiteral("softlight"))
        return BlendMode::SoftLight;
    if (lower == QStringLiteral("difference"))
        return BlendMode::Difference;
    return BlendMode::Normal;
}

QString stabilizeModeToString(StabilizeMode mode)
{
    if (mode == StabilizeMode::Keyframes)
        return QStringLiteral("keyframes");
    return QStringLiteral("bake");
}

StabilizeMode stabilizeModeFromString(const QString &mode)
{
    if (mode == QLatin1String("keyframes"))
        return StabilizeMode::Keyframes;
    return StabilizeMode::Bake;
}

} // namespace drift
