#pragma once

#include <QString>
#include <functional>

namespace drift {

// Transcodes source video into an All-Intra / Ultra-Short GOP H.264 proxy with downscaled resolution.
// Provides instantaneous timeline seeking (< 1ms) and 60 FPS playback on low-spec systems.
class ProxyRenderer
{
public:
    // Transcodes sourcePath into outPath at targetWidth x targetHeight.
    // onProgress is called periodically with progress in [0.0, 1.0]. Returning false cancels transcoding.
    static bool renderProxy(const QString &sourcePath, const QString &outPath,
                            int targetWidth, int targetHeight, QString *errorOut,
                            const std::function<bool(double)> &onProgress);
};

} // namespace drift
