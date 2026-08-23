#include "data/TreeNode.h"

bool TreeNode::isRoot() const
{
    return parentId == 0;
}

QString TreeNode::valueAsString() const
{
    return QString::number(value, 'g', 12);
}
