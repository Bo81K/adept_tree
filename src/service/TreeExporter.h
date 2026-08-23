#pragma once

#include <QJsonArray>
#include <QString>
#include <QVector>

#include "data/TreeNode.h"

class TreeRepository;

class TreeExporter
{
public:
    bool exportToFile(const QVector<TreeNode> &nodes,
                      const QString &filePath) const;

    bool importFromFile(const QString &filePath,
                        TreeRepository &repository,
                        bool clearExisting) const;

    QString lastError() const;

private:
    QJsonArray buildJsonArray(const QVector<TreeNode> &nodes) const;

    bool parseJsonArray(const QJsonArray &jsonArray,
                        TreeRepository &repository,
                        bool clearExisting) const;

    mutable QString lastErrorMessage;
};
