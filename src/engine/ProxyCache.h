#pragma once

#include <QHash>
#include <QMutex>
#include <QSize>
#include <QString>

namespace drift {

// Registry and on-disk index of lightweight forward editing proxies (>= 720p).
// Proxies use an All-Intra or ultra-short GOP H.264 profile scaled down to 540p or 720p,
// enabling zero-latency 60 FPS timeline scrubbing and playback even on low-end PCs.
class ProxyCache
{
public:
    struct Entry
    {
        QString proxyPath;
        qint64 sourceSize = 0;
        qint64 sourceMtimeMs = 0;
        int proxyWidth = 0;
        int proxyHeight = 0;
        qint64 lastAccessTimeMs = 0;
    };

    static ProxyCache &instance();

    // Returns the path of a valid proxy for sourcePath, or empty if none exists or if
    // the source file has changed since the proxy was generated.
    QString lookup(const QString &sourcePath) const;

    // Registers a newly generated proxy for sourcePath.
    void insert(const QString &sourcePath, const QString &proxyPath, int proxyWidth, int proxyHeight);

    // Removes any proxy entry for sourcePath.
    void invalidate(const QString &sourcePath);

    // Checks if video dimensions qualify for proxy generation (>= 720p: height >= 720 or width >= 1280).
    static bool isEligible(int width, int height);

    // Computes target proxy dimensions preserving aspect ratio (540p for <= 1080p, 720p for 4K).
    static QSize targetProxySize(int srcWidth, int srcHeight);

    // Reads the on-disk index, purging dead or outdated entries.
    void load();

    // Prunes the cache directory to maxBytes, removing oldest accessed proxies first.
    void sweep(qint64 maxBytes = kDefaultMaxBytes);

    // Base directory for forward proxies: <AppDataLocation>/proxies.
    static QString proxyCacheDir();

    // Generates a unique, clean path for a new proxy.
    static QString newProxyPath(const QString &sourcePath);

    static constexpr qint64 kDefaultMaxBytes = 15LL * 1024 * 1024 * 1024; // 15 GB

private:
    ProxyCache() = default;
    ~ProxyCache() = default;

    void saveLocked() const;

    mutable QMutex m_mutex;
    QHash<QString, Entry> m_entries; // absolute source path -> proxy entry
};

} // namespace drift
