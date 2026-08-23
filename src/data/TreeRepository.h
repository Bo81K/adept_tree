#pragma once

#include <QSqlDatabase>
#include <QString>
#include <QVector>

#include "data/TreeNode.h"

class TreeRepository
{
public:
    TreeRepository() = default;
    ~TreeRepository();

    bool open(const QString &databaseName,
              const QString &hostName,
              int port,
              const QString &userName,
              const QString &password);
    void close();
    bool isOpen() const;

    bool ensureSchema();
    bool clearAll();

    QVector<TreeNode> fetchAllNodes();
    qint64 insertNode(const TreeNode &node);
    bool updateNode(const TreeNode &node);
    bool removeNode(qint64 nodeId);

    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

    QString lastError() const;

private:
    QSqlDatabase database;
    QString connectionName;
    QString lastErrorMessage;
};
