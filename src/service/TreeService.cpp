#include "service/TreeService.h"
#include "data/TreeRepository.h"
#include "service/TreeExporter.h"

#include <QHash>

#include <utility>

TreeService::TreeService(const QSharedPointer<TreeRepository> &repository,
                         const QSharedPointer<TreeExporter> &exporter)
    : treeRepository(repository)
    , treeExporter(exporter)
{
}

TreeService::~TreeService() = default;

bool TreeService::reload()
{
    if (!treeRepository->isOpen())
    {
        lastErrorMessage = treeRepository->lastError();
        return false;
    }

    QVector<TreeNode> fetchedNodes = treeRepository->fetchAllNodes();

    cachedNodes = std::move(fetchedNodes);

    recalculateStatistics();

    return true;
}

const QVector<TreeNode> &TreeService::nodes() const
{
    return cachedNodes;
}

QVector<TreeNode> TreeService::visibleNodes(const TreeFilter &filter) const
{
    const QSet<qint64> ids = visibleNodeIds(filter);

    QVector<TreeNode> result;

    for (const TreeNode &node : cachedNodes)
    {
        if (ids.contains(node.id))
        {
            result.append(node);
        }
    }

    return result;
}

QSet<qint64> TreeService::visibleNodeIds(const TreeFilter &filter) const
{
    QSet<qint64> ids;

    if (filter.isEmpty())
    {
        for (const TreeNode &node : cachedNodes)
        {
            ids.insert(node.id);
        }

        return ids;
    }

    QHash<qint64, TreeNode> nodeById;
    QHash<qint64, qint64> parentById;

    for (const TreeNode &node : cachedNodes)
    {
        nodeById.insert(node.id, node);
        parentById.insert(node.id, node.parentId);
    }

    for (const TreeNode &node : cachedNodes)
    {
        if (!node.isLeaf)
        {
            continue;
        }

        if (!filter.matchesValue(node.value))
        {
            continue;
        }

        bool nameMatches = filter.matchesName(node.name);

        if (filter.searchInNodeNames && !nameMatches)
        {
            qint64 ancestorId = node.parentId;

            while (!nameMatches && ancestorId != 0)
            {
                const TreeNode ancestor = nodeById.value(ancestorId);

                if (ancestor.id == 0)
                {
                    break;
                }

                nameMatches = filter.matchesName(ancestor.name);
                ancestorId = ancestor.parentId;
            }
        }

        if (!nameMatches)
        {
            continue;
        }

        ids.insert(node.id);

        qint64 parentId = node.parentId;

        while (parentId != 0 && !ids.contains(parentId))
        {
            ids.insert(parentId);
            parentId = parentById.value(parentId, 0);
        }
    }

    return ids;
}

TreeStatistics TreeService::statistics() const
{
    return cachedStatistics;
}

TreeNode TreeService::findNode(qint64 nodeId) const
{
    for (const TreeNode &node : cachedNodes)
    {
        if (node.id == nodeId)
        {
            return node;
        }
    }

    return TreeNode();
}

qint64 TreeService::addNode(qint64 parentId, const QString &name)
{
    if (name.trimmed().isEmpty())
    {
        lastErrorMessage = QStringLiteral("Название узла не может быть пустым");
        return 0;
    }

    if (!isValidParent(parentId))
    {
        lastErrorMessage = QStringLiteral("Некорректный родительский узел");
        return 0;
    }

    TreeNode node;

    node.parentId = parentId;
    node.name = name.trimmed();
    node.isLeaf = false;

    const qint64 nodeId = treeRepository->insertNode(node);

    if (nodeId == 0)
    {
        lastErrorMessage = treeRepository->lastError();
        return 0;
    }

    reload();

    return nodeId;
}

qint64 TreeService::addLeaf(qint64 parentId, const QString &name, double value)
{
    if (name.trimmed().isEmpty())
    {
        lastErrorMessage = QStringLiteral("Название листа не может быть пустым");
        return 0;
    }

    if (!isValidParent(parentId))
    {
        lastErrorMessage = QStringLiteral("Некорректный родительский узел");
        return 0;
    }

    TreeNode node;

    node.parentId = parentId;
    node.name = name.trimmed();
    node.value = value;
    node.isLeaf = true;

    const qint64 nodeId = treeRepository->insertNode(node);

    if (nodeId == 0)
    {
        lastErrorMessage = treeRepository->lastError();
        return 0;
    }

    reload();

    return nodeId;
}

bool TreeService::updateLeaf(qint64 leafId, const QString &name, double value)
{
    if (name.trimmed().isEmpty())
    {
        lastErrorMessage = QStringLiteral("Название листа не может быть пустым");
        return false;
    }

    TreeNode leafNode = findNode(leafId);

    if (leafNode.id == 0 || !leafNode.isLeaf)
    {
        lastErrorMessage = QStringLiteral("Выбранный элемент не является листом");
        return false;
    }

    leafNode.name = name.trimmed();
    leafNode.value = value;

    if (!treeRepository->updateNode(leafNode))
    {
        lastErrorMessage = treeRepository->lastError();
        return false;
    }

    reload();

    return true;
}

bool TreeService::updateName(qint64 nodeId, const QString &name)
{
    if (name.trimmed().isEmpty())
    {
        lastErrorMessage = QStringLiteral("Название не может быть пустым");
        return false;
    }

    TreeNode node = findNode(nodeId);

    if (node.id == 0)
    {
        lastErrorMessage = QStringLiteral("Элемент не найден");
        return false;
    }

    node.name = name.trimmed();

    if (!treeRepository->updateNode(node))
    {
        lastErrorMessage = treeRepository->lastError();
        return false;
    }

    reload();

    return true;
}

bool TreeService::removeLeaf(qint64 leafId)
{
    const TreeNode leafNode = findNode(leafId);

    if (leafNode.id == 0 || !leafNode.isLeaf)
    {
        lastErrorMessage = QStringLiteral("Выбранный элемент не является листом");
        return false;
    }

    if (!treeRepository->removeNode(leafId))
    {
        lastErrorMessage = treeRepository->lastError();
        return false;
    }

    reload();

    return true;
}

bool TreeService::removeNodeWithLeaves(qint64 nodeId)
{
    const TreeNode node = findNode(nodeId);

    if (node.id == 0 || node.isLeaf)
    {
        lastErrorMessage = QStringLiteral("Выбранный элемент не является узлом");
        return false;
    }

    if (!treeRepository->removeNode(nodeId))
    {
        lastErrorMessage = treeRepository->lastError();
        return false;
    }

    reload();

    return true;
}

bool TreeService::exportToFile(const QString &filePath)
{
    if (!treeExporter->exportToFile(cachedNodes, filePath))
    {
        lastErrorMessage = treeExporter->lastError();
        return false;
    }

    return true;
}

bool TreeService::loadFromFile(const QString &filePath)
{
    if (!treeExporter->importFromFile(filePath, *treeRepository, true))
    {
        lastErrorMessage = treeExporter->lastError();
        return false;
    }

    return reload();
}

QString TreeService::lastError() const
{
    return lastErrorMessage;
}

bool TreeService::isValidParent(qint64 parentId) const
{
    if (parentId == 0)
    {
        return true;
    }

    const TreeNode parentNode = findNode(parentId);

    return parentNode.id != 0 && !parentNode.isLeaf;
}

void TreeService::recalculateStatistics()
{
    cachedStatistics = TreeStatistics();

    for (const TreeNode &node : cachedNodes)
    {
        if (node.isLeaf)
        {
            ++cachedStatistics.leafCount;

            if (!cachedStatistics.hasLeafValues)
            {
                cachedStatistics.hasLeafValues = true;
                cachedStatistics.maxValue = node.value;
                cachedStatistics.minValue = node.value;
            }
            else
            {
                if (node.value > cachedStatistics.maxValue)
                {
                    cachedStatistics.maxValue = node.value;
                }

                if (node.value < cachedStatistics.minValue)
                {
                    cachedStatistics.minValue = node.value;
                }
            }
        }
        else
        {
            ++cachedStatistics.nodeCount;
        }
    }
}
