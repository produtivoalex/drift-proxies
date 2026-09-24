#pragma once

#include <QObject>
#include <QVariantList>
#include <QString>
#include <QVariantMap>

namespace drift {

class ProjectPipelineManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList columns READ columns NOTIFY columnsChanged)

public:
    explicit ProjectPipelineManager(QObject *parent = nullptr);
    ~ProjectPipelineManager() override;

    QVariantList columns() const;

    Q_INVOKABLE void moveCard(const QString &cardId, const QString &targetColumnId, int targetIndex);
    Q_INVOKABLE void addCard(const QString &columnId, const QString &title, const QString &projectPath = QString());
    Q_INVOKABLE void removeCard(const QString &cardId);

    // Save and Load from global workspace
    Q_INVOKABLE void saveState() const;
    Q_INVOKABLE void loadState();

signals:
    void columnsChanged();

private:
    struct Card {
        QString id;
        QString title;
        QString projectPath; // Optional path to open the Drift project
        // Extensibility for thumbnails, descriptions, tags...
    };

    struct Column {
        QString id;
        QString name;
        QList<Card> cards;
    };

    QList<Column> m_columns;
    QString m_workspacePath;

    void initializeDefaultColumns();
};

} // namespace drift
