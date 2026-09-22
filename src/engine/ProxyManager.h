#pragma once

#include <QHash>
#include <QMutex>
#include <QObject>
#include <QQueue>
#include <QSet>
#include <QString>
#include <QThread>

namespace drift {

// Asynchronous background coordinator for generation and life-cycle of editing proxies.
// Dispatches proxy generation on a low-priority background thread so that timeline editing
// and GUI responsiveness are completely unaffected during transcoding.
class ProxyManager : public QObject
{
    Q_OBJECT

public:
    static ProxyManager &instance();

    // Requests proxy generation for sourcePath if it meets the resolution threshold (>= 720p).
    // If width/height are not provided, MediaProbe is used to query the stream dimensions.
    Q_INVOKABLE void requestProxy(const QString &sourcePath, int width = 0, int height = 0);

    // Checks if an active proxy is already cached and ready for playback.
    Q_INVOKABLE bool isProxyReady(const QString &sourcePath) const;

    // Checks if a proxy for sourcePath is currently being rendered in background.
    Q_INVOKABLE bool isGenerating(const QString &sourcePath) const;

    // Returns current generation progress [0.0, 1.0], or 0.0 if not generating.
    Q_INVOKABLE double progress(const QString &sourcePath) const;

    // Returns the proxy path for sourcePath if ready, or the original sourcePath if not.
    Q_INVOKABLE QString effectivePath(const QString &sourcePath) const;

    // Cancels proxy generation for a given source.
    Q_INVOKABLE void cancel(const QString &sourcePath);

    // Cancels all queued and active proxy generation jobs.
    Q_INVOKABLE void cancelAll();

signals:
    void proxyStarted(const QString &sourcePath);
    void proxyProgress(const QString &sourcePath, double progress);
    void proxyReady(const QString &sourcePath, const QString &proxyPath);
    void proxyFailed(const QString &sourcePath, const QString &error);

private:
    explicit ProxyManager(QObject *parent = nullptr);
    ~ProxyManager() override;

    struct Job
    {
        QString sourcePath;
        int width = 0;
        int height = 0;
    };

    void processNextJob();

    mutable QMutex m_mutex;
    QQueue<Job> m_queue;
    QSet<QString> m_pending;
    QString m_activeSource;
    double m_activeProgress = 0.0;
    bool m_cancelRequested = false;
    QThread m_workerThread;
};

} // namespace drift
