#pragma once

#include <QVector>
#include <QWidget>

#include "data/TreeNode.h"

class QTreeWidget;
class QTreeWidgetItem;

class TreeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TreeWidget(QWidget *parent = nullptr);

    void setTreeData(const QVector<TreeNode> &nodes);
    void clearTree();
    void editSelectedName();
    qint64 selectedNodeId() const;

signals:
    void signalItemEdited(qint64 nodeId, int column, const QString &newValue);

private:
    void initializeUi();
    QTreeWidgetItem *createItem(const TreeNode &node) const;

    QTreeWidget *itemsTreeWidget = nullptr;
};
