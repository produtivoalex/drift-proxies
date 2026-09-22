#include "ProxyManager.h"

#include "MediaProbe.h"
#include "ProxyCache.h"
#include "ProxyRenderer.h"

#include <QFileInfo>
#include <QThreadPool>

namespace drift {

ProxyManager &ProxyManager::instance()
{
    static ProxyManager manager;
    return manager;
}

ProxyManager::ProxyManager(QObject *parent)
    : QObject(parent)
{
}

ProxyManager::~ProxyManager()
{
    cancelAll();
}

bool ProxyManager::isProxyReady(const QString &sourcePath) const
{
    if (sourcePath.isEmpty())
        return false;
    return !ProxyCache::instance().lookup(sourcePath).isEmpty();
}

bool ProxyManager::isGenerating(const QString &sourcePath) const
{
    QMutexLocker lock(&m_mutex);
    return m_pending.contains(sourcePath) || (m_activeSource == sourcePath);
}

double ProxyManager::progress(const QString &sourcePath) const
{
    QMutexLocker lock(&m_mutex);
    if (m_activeSource == sourcePath)
        return m_activeProgress;
    return 0.0;
}

QString ProxyManager::effectivePath(const QString &sourcePath) const
{
    if (sourcePath.isEmpty())
        return {};

    const QString proxy = ProxyCache::instance().lookup(sourcePath);
    return proxy.isEmpty() ? sourcePath : proxy;
}

void ProxyManager::requestProxy(const QString &sourcePath, int width, int height)
{
    if (sourcePath.isEmpty())
        return;

    // Check if proxy already exists and is valid
    if (isProxyReady(sourcePath))
        return;

    // Check if already queued or active
    {
        QMutexLocker lock(&m_mutex);
        if (m_pending.contains(sourcePath) || m_activeSource == sourcePath)
            return;
    }

    // Determine dimensions if not provided
    if (width <= 0 || height <= 0) {
        const MediaInfo info = MediaProbe::probe(sourcePath);
        for (const StreamInfo &s : info.streams) {
            if (s.type == StreamInfo::Type::Video) {
                width = s.width;
                height = s.height;
                break;
            }
        }
    }

    // Check if resolution meets the threshold (>= 720p)
    if (!ProxyCache::isEligible(width, height))
        return;

    {
        QMutexLocker lock(&m_mutex);
        m_queue.enqueue(Job{sourcePath, width, height});
        m_pending.insert(sourcePath);
    }

    processNextJob();
}

void ProxyManager::cancel(const QString &sourcePath)
{
    QMutexLocker lock(&m_mutex);
    m_pending.remove(sourcePath);
    for (int i = m_queue.size() - 1; i >= 0; --i) {
        if (m_queue.at(i).sourcePath == sourcePath)
            m_queue.removeAt(i);
    }
    if (m_activeSource == sourcePath) {
        m_cancelRequested = true;
    }
}

void ProxyManager::cancelAll()
{
    QMutexLocker lock(&m_mutex);
    m_queue.clear();
    m_pending.clear();
    m_cancelRequested = true;
}

void ProxyManager::processNextJob()
{
    QMutexLocker lock(&m_mutex);
    if (!m_activeSource.isEmpty() || m_queue.isEmpty())
        return;

    const Job job = m_queue.dequeue();
    m_activeSource = job.sourcePath;
    m_activeProgress = 0.0;
    m_cancelRequested = false;

    lock.unlock();

    emit proxyStarted(job.sourcePath);

    // Run transcoding on Qt background threadpool with lowest priority
    QThreadPool::globalInstance()->start([this, job]() {
        const QSize targetSize = ProxyCache::targetProxySize(job.width, job.height);
        const QString outPath = ProxyCache::newProxyPath(job.sourcePath);
        QString error;

        auto progressCallback = [this, job](double prog) -> bool {
            QMutexLocker lock(&m_mutex);
            if (m_cancelRequested)
                return false;
            m_activeProgress = prog;
            lock.unlock();

            QMetaObject::invokeMethod(this, [this, path = job.sourcePath, prog]() {
                emit proxyProgress(path, prog);
            }, Qt::QueuedConnection);

            return true;
        };

        const bool success = ProxyRenderer::renderProxy(
            job.sourcePath, outPath, targetSize.width(), targetSize.height(),
            &error, progressCallback);

        QMutexLocker lock(&m_mutex);
        const bool wasCancelled = m_cancelRequested;
        m_pending.remove(job.sourcePath);
        m_activeSource.clear();
        m_activeProgress = 0.0;
        m_cancelRequested = false;
        lock.unlock();

        if (success && !wasCancelled) {
            ProxyCache::instance().insert(job.sourcePath, outPath, targetSize.width(), targetSize.height());
            QMetaObject::invokeMethod(this, [this, src = job.sourcePath, proxy = outPath]() {
                emit proxyReady(src, proxy);
            }, Qt::QueuedConnection);
        } else if (!wasCancelled) {
            QMetaObject::invokeMethod(this, [this, src = job.sourcePath, err = error]() {
                emit proxyFailed(src, err);
            }, Qt::QueuedConnection);
        }

        // Process subsequent queued jobs
        QMetaObject::invokeMethod(this, &ProxyManager::processNextJob, Qt::QueuedConnection);
    });
}

} // namespace drift
