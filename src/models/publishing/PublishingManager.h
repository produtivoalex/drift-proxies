// src/models/publishing/PublishingManager.h
// Fase 6A — Publicação Direta nas Redes Sociais
// Suporte a YouTube, TikTok, Instagram Reels e Twitter/X com OAuth2 e agendamento.
#pragma once

#include <QDateTime>
#include <QList>
#include <QMap>
#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTcpServer>
#include <QUrl>
#include <QVariantMap>

namespace drift {

enum class SocialPlatform {
    YouTube = 0,
    TikTok = 1,
    InstagramReels = 2,
    TwitterX = 3
};

struct PublishJob {
    QString id;
    SocialPlatform platform = SocialPlatform::YouTube;
    QString videoFilePath;
    QString title;
    QString description;
    QStringList tags;
    QString thumbnailPath;
    QDateTime scheduledAt; // Invalid/null = publish immediately
    bool isPublic = true;
    QString playlistId;    // YouTube only
    QString language = QStringLiteral("pt-BR");
};

struct PublishResult {
    bool success = false;
    QString jobId;
    SocialPlatform platform = SocialPlatform::YouTube;
    QString videoUrl;
    QString platformVideoId;
    QString error;
};

class PublishingManager : public QObject {
    Q_OBJECT

public:
    explicit PublishingManager(QObject *parent = nullptr);
    ~PublishingManager() override;

    // ── Autenticação OAuth2 ──────────────────────────────────────────────────
    Q_INVOKABLE void startAuth(int platformIndex);
    Q_INVOKABLE void disconnectPlatform(int platformIndex);
    Q_INVOKABLE bool isAuthenticated(int platformIndex) const;
    Q_INVOKABLE QString accountName(int platformIndex) const;

    // ── Publicação e Agendamento ─────────────────────────────────────────────
    // Publica ou agenda um vídeo
    Q_INVOKABLE QString publishVideo(int platformIndex,
                                     const QString &videoPath,
                                     const QString &title,
                                     const QString &description,
                                     const QStringList &tags,
                                     const QString &thumbnailPath = QString(),
                                     const QDateTime &scheduledAt = QDateTime(),
                                     bool isPublic = true);

    Q_INVOKABLE void cancelScheduledJob(const QString &jobId);
    Q_INVOKABLE QVariantList scheduledJobs() const;
    Q_INVOKABLE QVariantList availablePlatforms() const;

signals:
    void authSuccess(int platformIndex, const QString &accountName);
    void authFailed(int platformIndex, const QString &error);
    void authStatusChanged();
    void publishProgress(const QString &jobId, double fraction, const QString &status);
    void publishFinished(const QString &jobId, bool success, const QString &videoUrl, const QString &error);
    void scheduledJobsChanged();

private:
    void setupOAuthServer();
    void handleIncomingConnection();
    void exchangeCodeForToken(SocialPlatform platform, const QString &code);
    void executeUpload(const PublishJob &job);

    // Platform-specific uploads
    void uploadToYouTube(const PublishJob &job);
    void uploadToTikTok(const PublishJob &job);
    void uploadToInstagram(const PublishJob &job);

    QNetworkAccessManager m_net;
    QTcpServer *m_authServer = nullptr;
    SocialPlatform m_pendingAuthPlatform = SocialPlatform::YouTube;

    QMap<SocialPlatform, QString> m_tokens;
    QMap<SocialPlatform, QString> m_accountNames;
    QList<PublishJob> m_jobs;
    quint16 m_authPort = 8374;
};

} // namespace drift
