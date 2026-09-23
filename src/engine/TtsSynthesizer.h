#pragma once

#include "core/SubtitleCue.h"
#include <QString>
#include <QList>
#include <QVariantMap>

namespace drift {

struct TtsVoiceInfo {
    QString id;
    QString name;
    QString lang;
    QString gender;
    QString vibeTag;
    QString description;
    bool isFeatured = false;
    bool isNeural = false;
    double defaultRate = 1.0;
    double defaultPitch = 1.0;

    QVariantMap toVariantMap() const {
        QVariantMap map;
        map[QStringLiteral("id")] = id;
        map[QStringLiteral("name")] = name;
        map[QStringLiteral("lang")] = lang;
        map[QStringLiteral("gender")] = gender;
        map[QStringLiteral("vibeTag")] = vibeTag;
        map[QStringLiteral("description")] = description;
        map[QStringLiteral("isFeatured")] = isFeatured;
        map[QStringLiteral("isNeural")] = isNeural;
        map[QStringLiteral("defaultRate")] = defaultRate;
        map[QStringLiteral("defaultPitch")] = defaultPitch;
        return map;
    }
};

struct TtsSynthesizeResult {
    bool ok = false;
    QString filePath;
    double durationSeconds = 0.0;
    QList<SubtitleCue> cues;
    QString error;
};

class TtsSynthesizer {
public:
    static TtsSynthesizer &instance();

    // Returns all detected system voices + local neural voices
    QList<TtsVoiceInfo> availableVoices();

    // Synthesizes speech to a standard PCM WAV file in the local cache
    TtsSynthesizeResult synthesize(const QString &text,
                                  const QString &voiceId = QString(),
                                  double rate = 1.0,
                                  double pitch = 1.0);

private:
    TtsSynthesizer();
    QList<SubtitleCue> generateCuesForText(const QString &text, double totalDurationSec);
    QString ttsOutputDir() const;
    double measureWavDuration(const QString &wavPath) const;
};

} // namespace drift
