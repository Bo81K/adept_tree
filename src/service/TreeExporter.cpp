#include "service/TreeExporter.h"
#include "data/TreeRepository.h"

#include <QFile>
#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QMultiHash>

#include <functional>

bool TreeExporter::exportToFile(const QVector<TreeNode> &nodes,
                                const QString &filePath) const
{
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        lastErrorMessage = file.errorString();
        return false;
    }

    const QJsonDocument document(buildJsonArray(nodes));
    const QByteArray jsonData = document.toJson(QJsonDocument::Indented);

    if (file.write(jsonData) != static_cast<qint64>(jsonData.size()))
    {
        lastErrorMessage = file.errorString();
        return false;
    }

    return true;
}

bool TreeExporter::importFromFile(const QString &filePath,
                                  TreeRepository &repository,
                                  bool clearExisting) const
{
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly))
    {
        lastErrorMessage = file.errorString();
        return false;
    }

    QJsonParseError parseError;

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);

    if (parseError.error != QJsonParseError::NoError)
    {
        lastErrorMessage = parseError.errorString();
        return false;
    }

    if (!document.isArray())
    {
        lastErrorMessage = QStringLiteral("Ожидался JSON-массив");
        return false;
    }

    return parseJsonArray(document.array(), repository, clearExisting);
}

QString TreeExporter::lastError() const
{
    return lastErrorMessage;
}

QJsonArray TreeExporter::buildJsonArray(const QVector<TreeNode> &nodes) const
{
    QMultiHash<qint64, TreeNode> childrenByParent;

    for (const TreeNode &node : nodes)
    {
        childrenByParent.insert(node.parentId, node);
    }

    QVector<TreeNode> orderedNodes;
    QHash<qint64, int> indexById;

    std::function<void(qint64)> appendChildren = [&](qint64 parentId)
    {
        const QList<TreeNode> childNodes = childrenByParent.values(parentId);

        for (const TreeNode &childNode : childNodes)
        {
            const int childIndex = static_cast<int>(orderedNodes.size());

            indexById.insert(childNode.id, childIndex);
            orderedNodes.append(childNode);
            appendChildren(childNode.id);
        }
    };

    appendChildren(0);

    QJsonArray jsonArray;

    for (int index = 0; index < static_cast<int>(orderedNodes.size()); ++index)
    {
        const TreeNode &node = orderedNodes.at(index);

        QJsonObject jsonObject;

        int parentIndex = -1;

        if (node.parentId != 0)
        {
            parentIndex = indexById.value(node.parentId, -1);
        }

        jsonObject.insert(QStringLiteral("index"), index);
        jsonObject.insert(QStringLiteral("parentIndex"), parentIndex);
        jsonObject.insert(QStringLiteral("name"), node.name);
        jsonObject.insert(QStringLiteral("isLeaf"), node.isLeaf);

        if (node.isLeaf)
        {
            jsonObject.insert(QStringLiteral("value"), node.value);
        }

        jsonArray.append(jsonObject);
    }

    return jsonArray;
}

bool TreeExporter::parseJsonArray(const QJsonArray &jsonArray,
                                  TreeRepository &repository,
                                  bool clearExisting) const
{
    if (!repository.beginTransaction())
    {
        lastErrorMessage = repository.lastError();
        return false;
    }

    if (clearExisting && !repository.clearAll())
    {
        lastErrorMessage = repository.lastError();
        repository.rollbackTransaction();
        return false;
    }

    QHash<int, qint64> realIdByIndex;

    for (int arrayIndex = 0; arrayIndex < static_cast<int>(jsonArray.size()); ++arrayIndex)
    {
        const QJsonObject jsonObject = jsonArray.at(arrayIndex).toObject();

        const int itemIndex = jsonObject.value(QStringLiteral("index")).toInt(arrayIndex);
        const int parentIndex = jsonObject.value(QStringLiteral("parentIndex")).toInt(-1);

        qint64 parentId = 0;

        if (parentIndex != -1)
        {
            if (!realIdByIndex.contains(parentIndex))
            {
                lastErrorMessage = QStringLiteral("Родительский узел не определён до дочернего");
                repository.rollbackTransaction();
                return false;
            }

            parentId = realIdByIndex.value(parentIndex);
        }

        TreeNode node;

        node.parentId = parentId;
        node.name = jsonObject.value(QStringLiteral("name")).toString();
        node.isLeaf = jsonObject.value(QStringLiteral("isLeaf")).toBool();
        node.value = jsonObject.value(QStringLiteral("value")).toDouble();

        const qint64 nodeId = repository.insertNode(node);

        if (nodeId == 0)
        {
            lastErrorMessage = repository.lastError();
            repository.rollbackTransaction();
            return false;
        }

        realIdByIndex.insert(itemIndex, nodeId);
    }

    if (!repository.commitTransaction())
    {
        lastErrorMessage = repository.lastError();
        repository.rollbackTransaction();
        return false;
    }

    return true;
}
