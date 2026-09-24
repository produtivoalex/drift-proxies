#pragma once

#include <QObject>
#include <QString>
#include <atomic>

namespace drift {

class Project;

class WizardEngine : public QObject {
    Q_OBJECT
public:
    explicit WizardEngine(QObject *parent = nullptr);
    ~WizardEngine() override;

    // Cancela qualquer geração em andamento
    void cancel();

    // Inicia a geração do vídeo baseado no roteiro.
    // O progresso e a conclusão serão reportados via signals.
    void generateTimeline(const QString &script, const QString &vibe, const QString &voiceId, Project *project);

signals:
    void progressChanged(double fraction, const QString &status);
    void finished(bool success, const QString &error);

private:
    void processAsync(QString script, QString vibe, QString voiceId, Project *project);

    std::atomic<bool> m_cancel{false};
};

} // namespace drift
