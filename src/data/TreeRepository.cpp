#include "data/TreeRepository.h"

#include <QDateTime>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

TreeRepository::~TreeRepository()
{
    close();
}

bool TreeRepository::open(const QString &databaseName,
                          const QString &hostName,
                          int port,
                          const QString &userName,
                          const QString &password)
{
    close();

    connectionName = QStringLiteral("adept_tree_connection_");
    connectionName += QString::number(QDateTime::currentMSecsSinceEpoch());

    database = QSqlDatabase::addDatabase(QStringLiteral("QPSQL"), connectionName);

    if (!database.isValid())
    {
        lastErrorMessage = QStringLiteral("Драйвер QPSQL недоступен");
        return false;
    }

    database.setDatabaseName(databaseName);
    database.setHostName(hostName);
    database.setPort(port);
    database.setUserName(userName);
    database.setPassword(password);

    if (!database.open())
    {
        lastErrorMessage = database.lastError().text();
        return false;
    }

    return true;
}

void TreeRepository::close()
{
    if (!database.isValid())
    {
        return;
    }

    if (database.isOpen())
    {
        database.close();
    }

    const QString removedConnectionName = connectionName;

    database = QSqlDatabase();
    QSqlDatabase::removeDatabase(removedConnectionName);
    connectionName.clear();
}

bool TreeRepository::isOpen() const
{
    return database.isValid() && database.isOpen();
}

bool TreeRepository::ensureSchema()
{
    QSqlQuery query(database);

    const QString createTableQuery = QStringLiteral("CREATE TABLE IF NOT EXISTS tree_nodes (")
        + QStringLiteral("id BIGSERIAL PRIMARY KEY, ")
        + QStringLiteral("parent_id BIGINT NULL REFERENCES tree_nodes(id) ON DELETE CASCADE, ")
        + QStringLiteral("name TEXT NOT NULL, ")
        + QStringLiteral("value DOUBLE PRECISION NULL, ")
        + QStringLiteral("is_leaf BOOLEAN NOT NULL DEFAULT FALSE)");

    if (!query.exec(createTableQuery))
    {
        lastErrorMessage = query.lastError().text();
        return false;
    }

    const QString createIndexQuery = QStringLiteral(
        "CREATE INDEX IF NOT EXISTS tree_nodes_parent_id_idx ON tree_nodes(parent_id)");

    if (!query.exec(createIndexQuery))
    {
        lastErrorMessage = query.lastError().text();
        return false;
    }

    return true;
}

bool TreeRepository::clearAll()
{
    QSqlQuery query(database);

    const QString clearQuery = QStringLiteral("TRUNCATE TABLE tree_nodes RESTART IDENTITY CASCADE");

    if (!query.exec(clearQuery))
    {
        lastErrorMessage = query.lastError().text();
        return false;
    }

    return true;
}

QVector<TreeNode> TreeRepository::fetchAllNodes()
{
    QVector<TreeNode> nodes;

    QSqlQuery query(database);

    const QString selectQuery = QStringLiteral(
        "SELECT id, parent_id, name, value, is_leaf "
        "FROM tree_nodes "
        "ORDER BY id");

    if (!query.exec(selectQuery))
    {
        lastErrorMessage = query.lastError().text();
        return nodes;
    }

    while (query.next())
    {
        TreeNode node;

        node.id = query.value(0).toLongLong();
        node.parentId = query.value(1).isNull() ? 0LL : query.value(1).toLongLong();
        node.name = query.value(2).toString();
        node.value = query.value(3).isNull() ? 0.0 : query.value(3).toDouble();
        node.isLeaf = query.value(4).toBool();

        nodes.append(node);
    }

    return nodes;
}

qint64 TreeRepository::insertNode(const TreeNode &node)
{
    QSqlQuery query(database);

    const QString insertQuery = QStringLiteral(
        "INSERT INTO tree_nodes (parent_id, name, value, is_leaf) "
        "VALUES (:parent_id, :name, :value, :is_leaf) "
        "RETURNING id");

    query.prepare(insertQuery);

    if (node.parentId == 0)
    {
        query.bindValue(QStringLiteral(":parent_id"), QVariant());
    }
    else
    {
        query.bindValue(QStringLiteral(":parent_id"), node.parentId);
    }

    query.bindValue(QStringLiteral(":name"), node.name);

    if (node.isLeaf)
    {
        query.bindValue(QStringLiteral(":value"), node.value);
    }
    else
    {
        query.bindValue(QStringLiteral(":value"), QVariant());
    }

    query.bindValue(QStringLiteral(":is_leaf"), node.isLeaf);

    if (!query.exec())
    {
        lastErrorMessage = query.lastError().text();
        return 0;
    }

    if (!query.next())
    {
        lastErrorMessage = QStringLiteral("Не удалось получить идентификатор добавленной записи");
        return 0;
    }

    return query.value(0).toLongLong();
}

bool TreeRepository::updateNode(const TreeNode &node)
{
    QSqlQuery query(database);

    const QString updateQuery = QStringLiteral("UPDATE tree_nodes SET parent_id = :parent_id, ")
        + QStringLiteral("name = :name, value = :value, is_leaf = :is_leaf ")
        + QStringLiteral("WHERE id = :id");

    query.prepare(updateQuery);

    if (node.parentId == 0)
    {
        query.bindValue(QStringLiteral(":parent_id"), QVariant());
    }
    else
    {
        query.bindValue(QStringLiteral(":parent_id"), node.parentId);
    }

    query.bindValue(QStringLiteral(":name"), node.name);

    if (node.isLeaf)
    {
        query.bindValue(QStringLiteral(":value"), node.value);
    }
    else
    {
        query.bindValue(QStringLiteral(":value"), QVariant());
    }

    query.bindValue(QStringLiteral(":is_leaf"), node.isLeaf);
    query.bindValue(QStringLiteral(":id"), node.id);

    if (!query.exec())
    {
        lastErrorMessage = query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() <= 0)
    {
        lastErrorMessage = QStringLiteral("Запись не найдена");
        return false;
    }

    return true;
}

bool TreeRepository::removeNode(qint64 nodeId)
{
    QSqlQuery query(database);

    query.prepare(QStringLiteral("DELETE FROM tree_nodes WHERE id = :id"));
    query.bindValue(QStringLiteral(":id"), nodeId);

    if (!query.exec())
    {
        lastErrorMessage = query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() <= 0)
    {
        lastErrorMessage = QStringLiteral("Запись не найдена");
        return false;
    }

    return true;
}

bool TreeRepository::beginTransaction()
{
    if (!database.transaction())
    {
        lastErrorMessage = database.lastError().text();
        return false;
    }

    return true;
}

bool TreeRepository::commitTransaction()
{
    if (!database.commit())
    {
        lastErrorMessage = database.lastError().text();
        return false;
    }

    return true;
}

bool TreeRepository::rollbackTransaction()
{
    if (!database.rollback())
    {
        lastErrorMessage = database.lastError().text();
        return false;
    }

    return true;
}

QString TreeRepository::lastError() const
{
    return lastErrorMessage;
}
