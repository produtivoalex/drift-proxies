#include "ProjectPipelineManager.h"

#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUuid>

namespace drift {

ProjectPipelineManager::ProjectPipelineManager(QObject *parent) : QObject(parent)
{
    // Global workspace path for the Kanban board
    QString appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataDir);
    m_workspacePath = appDataDir + "/kanban_state.json";

    loadState();
    
    if (m_columns.isEmpty()) {
        initializeDefaultColumns();
        saveState();
    }
}

ProjectPipelineManager::~ProjectPipelineManager()
{
}

void ProjectPipelineManager::initializeDefaultColumns()
{
    m_columns.clear();
    
    Column idea{"col_idea", tr("Ideia"), {}};
    Column scripting{"col_scripting", tr("Roteirizando"), {}};
    Column ready{"col_ready", tr("Pronto para Edição"), {}};
    Column rendering{"col_rendering", tr("Renderizando"), {}};
    Column published{"col_published", tr("Publicado"), {}};

    m_columns.append(idea);
    m_columns.append(scripting);
    m_columns.append(ready);
    m_columns.append(rendering);
    m_columns.append(published);

    // Some dummy data to show off
    addCard("col_idea", "Vídeo: 5 Segredos de Finanças");
    addCard("col_scripting", "Shorts: Mistério do Faraó");
}

QVariantList ProjectPipelineManager::columns() const
{
    QVariantList list;
    for (const Column &col : m_columns) {
        QVariantMap colMap;
        colMap["id"] = col.id;
        colMap["name"] = col.name;

        QVariantList cardsList;
        for (const Card &card : col.cards) {
            QVariantMap cardMap;
            cardMap["id"] = card.id;
            cardMap["title"] = card.title;
            cardMap["projectPath"] = card.projectPath;
            cardsList.append(cardMap);
        }
        colMap["cards"] = cardsList;
        list.append(colMap);
    }
    return list;
}

void ProjectPipelineManager::moveCard(const QString &cardId, const QString &targetColumnId, int targetIndex)
{
    Card targetCard;
    bool found = false;

    // Remove from source
    for (int i = 0; i < m_columns.size(); ++i) {
        for (int j = 0; j < m_columns[i].cards.size(); ++j) {
            if (m_columns[i].cards[j].id == cardId) {
                targetCard = m_columns[i].cards.takeAt(j);
                found = true;
                break;
            }
        }
        if (found) break;
    }

    if (!found) return;

    // Insert to target
    for (int i = 0; i < m_columns.size(); ++i) {
        if (m_columns[i].id == targetColumnId) {
            if (targetIndex < 0 || targetIndex > m_columns[i].cards.size()) {
                targetIndex = m_columns[i].cards.size();
            }
            m_columns[i].cards.insert(targetIndex, targetCard);
            break;
        }
    }

    saveState();
    emit columnsChanged();
}

void ProjectPipelineManager::addCard(const QString &columnId, const QString &title, const QString &projectPath)
{
    for (int i = 0; i < m_columns.size(); ++i) {
        if (m_columns[i].id == columnId) {
            Card card;
            card.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
            card.title = title;
            card.projectPath = projectPath;
            m_columns[i].cards.append(card);
            
            saveState();
            emit columnsChanged();
            return;
        }
    }
}

void ProjectPipelineManager::removeCard(const QString &cardId)
{
    for (int i = 0; i < m_columns.size(); ++i) {
        for (int j = 0; j < m_columns[i].cards.size(); ++j) {
            if (m_columns[i].cards[j].id == cardId) {
                m_columns[i].cards.removeAt(j);
                
                saveState();
                emit columnsChanged();
                return;
            }
        }
    }
}

void ProjectPipelineManager::saveState() const
{
    QJsonArray columnsArray;
    for (const Column &col : m_columns) {
        QJsonObject colObj;
        colObj["id"] = col.id;
        colObj["name"] = col.name;

        QJsonArray cardsArray;
        for (const Card &card : col.cards) {
            QJsonObject cardObj;
            cardObj["id"] = card.id;
            cardObj["title"] = card.title;
            cardObj["projectPath"] = card.projectPath;
            cardsArray.append(cardObj);
        }
        colObj["cards"] = cardsArray;
        columnsArray.append(colObj);
    }

    QJsonObject root;
    root["columns"] = columnsArray;

    QFile file(m_workspacePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(root).toJson());
    }
}

void ProjectPipelineManager::loadState()
{
    QFile file(m_workspacePath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) return;

    QJsonObject root = doc.object();
    QJsonArray columnsArray = root["columns"].toArray();

    m_columns.clear();

    for (int i = 0; i < columnsArray.size(); ++i) {
        QJsonObject colObj = columnsArray[i].toObject();
        Column col;
        col.id = colObj["id"].toString();
        col.name = colObj["name"].toString();

        QJsonArray cardsArray = colObj["cards"].toArray();
        for (int j = 0; j < cardsArray.size(); ++j) {
            QJsonObject cardObj = cardsArray[j].toObject();
            Card card;
            card.id = cardObj["id"].toString();
            card.title = cardObj["title"].toString();
            card.projectPath = cardObj["projectPath"].toString();
            col.cards.append(card);
        }
        m_columns.append(col);
    }

    emit columnsChanged();
}

} // namespace drift
