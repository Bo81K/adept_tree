#include "ui/TreeWidget.h"

#include <QAbstractItemView>
#include <QHeaderView>
#include <QList>
#include <QMultiHash>
#include <QStringList>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <functional>

TreeWidget::TreeWidget(QWidget *parent)
    : QWidget(parent)
{
    initializeUi();
}

void TreeWidget::initializeUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    itemsTreeWidget = new QTreeWidget(this);
    itemsTreeWidget->setColumnCount(2);
    itemsTreeWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QStringList headerLabels{QStringLiteral("Название"), QStringLiteral("Значение")};
    itemsTreeWidget->setHeaderLabels(headerLabels);

    itemsTreeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    itemsTreeWidget->header()->setStretchLastSection(true);

    mainLayout->addWidget(itemsTreeWidget);

    connect(itemsTreeWidget,
            &QTreeWidget::itemDoubleClicked,
            this,
            [this](QTreeWidgetItem *item, int column)
            {
                if (item == nullptr)
                {
                    return;
                }

                if (column == 0)
                {
                    itemsTreeWidget->editItem(item, 0);
                }
                else if (column == 1)
                {
                    const bool isLeaf = item->data(0, Qt::UserRole + 1).toBool();

                    if (isLeaf)
                    {
                        itemsTreeWidget->editItem(item, 1);
                    }
                }
            });

    connect(itemsTreeWidget,
            &QTreeWidget::itemChanged,
            this,
            [this](QTreeWidgetItem *item, int column)
            {
                if (item == nullptr)
                {
                    return;
                }

                const qint64 nodeId = item->data(0, Qt::UserRole).toLongLong();

                if (nodeId == 0)
                {
                    return;
                }

                if (column == 1)
                {
                    const bool isLeaf = item->data(0, Qt::UserRole + 1).toBool();

                    if (!isLeaf)
                    {
                        return;
                    }
                }

                emit signalItemEdited(nodeId, column, item->text(column));
            });
}

void TreeWidget::setTreeData(const QVector<TreeNode> &nodes)
{
    clearTree();

    itemsTreeWidget->blockSignals(true);

    QMultiHash<qint64, TreeNode> childrenByParent;

    for (const TreeNode &node : nodes)
    {
        childrenByParent.insert(node.parentId, node);
    }

    std::function<void(qint64, QTreeWidgetItem *)> appendChildren =
        [&](qint64 parentId, QTreeWidgetItem *parentItem)
    {
        const QList<TreeNode> childNodes = childrenByParent.values(parentId);

        for (const TreeNode &childNode : childNodes)
        {
            QTreeWidgetItem *item = createItem(childNode);

            if (parentItem == nullptr)
            {
                itemsTreeWidget->addTopLevelItem(item);
            }
            else
            {
                parentItem->addChild(item);
            }

            appendChildren(childNode.id, item);
        }
    };

    appendChildren(0, nullptr);

    itemsTreeWidget->expandAll();

    itemsTreeWidget->blockSignals(false);
}

void TreeWidget::clearTree()
{
    itemsTreeWidget->blockSignals(true);

    itemsTreeWidget->clear();

    itemsTreeWidget->blockSignals(false);
}

void TreeWidget::editSelectedName()
{
    QTreeWidgetItem *item = itemsTreeWidget->currentItem();

    if (item == nullptr)
    {
        return;
    }

    if ((item->flags() & Qt::ItemIsEditable) == 0)
    {
        return;
    }

    itemsTreeWidget->editItem(item, 0);
}

qint64 TreeWidget::selectedNodeId() const
{
    QTreeWidgetItem *item = itemsTreeWidget->currentItem();

    if (item == nullptr)
    {
        return 0;
    }

    return item->data(0, Qt::UserRole).toLongLong();
}

QTreeWidgetItem *TreeWidget::createItem(const TreeNode &node) const
{
    QTreeWidgetItem *item = new QTreeWidgetItem();

    item->setText(0, node.name);

    if (node.isLeaf)
    {
        item->setText(1, node.valueAsString());
    }
    else
    {
        item->setText(1, QString());
    }

    item->setData(0, Qt::UserRole, node.id);
    item->setData(0, Qt::UserRole + 1, node.isLeaf);

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;

    item->setFlags(flags);

    return item;
}
