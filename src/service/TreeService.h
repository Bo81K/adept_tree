#pragma once

#include <QSet>
#include <QSharedPointer>
#include <QString>
#include <QVector>

#include "data/TreeNode.h"
#include "service/TreeFilter.h"

class TreeRepository;
class TreeExporter;

struct TreeStatistics
{
    int nodeCount = 0;
    int leafCount = 0;
    double maxValue = 0.0;
    double minValue = 0.0;
    bool hasLeafValues = false;
};

class TreeService
{
public:
    TreeService(const QSharedPointer<TreeRepository> &repository,
                const QSharedPointer<TreeExporter> &exporter);

    ~TreeService();

    bool reload();

    const QVector<TreeNode> &nodes() const;
    QVector<TreeNode> visibleNodes(const TreeFilter &filter) const;
    QSet<qint64> visibleNodeIds(const TreeFilter &filter) const;

    TreeStatistics statistics() const;
    TreeNode findNode(qint64 nodeId) const;

    qint64 addNode(qint64 parentId, const QString &name);
    qint64 addLeaf(qint64 parentId, const QString &name, double value);

    bool updateLeaf(qint64 leafId, const QString &name, double value);
    bool updateName(qint64 nodeId, const QString &name);
    bool removeLeaf(qint64 leafId);
    bool removeNodeWithLeaves(qint64 nodeId);

    bool exportToFile(const QString &filePath);
    bool loadFromFile(const QString &filePath);

    QString lastError() const;

private:
    bool isValidParent(qint64 parentId) const;
    void recalculateStatistics();

    QSharedPointer<TreeRepository> treeRepository;
    QSharedPointer<TreeExporter> treeExporter;

    QVector<TreeNode> cachedNodes;
    TreeStatistics cachedStatistics;
    QString lastErrorMessage;
};
