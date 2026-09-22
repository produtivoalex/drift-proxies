#pragma once

#include <QList>
#include <QString>
#include <QVariantMap>

struct SfxItem
{
    QString id;
    QString label;
    QString category;
    double durationSeconds = 0.0;
    QString icon;
};

struct SfxCategory
{
    QString id;
    QString label;
};

const QList<SfxCategory> &sfxCategories();
const QList<SfxItem> &sfxCatalog();
QString sfxFilePath(const QString &id);
