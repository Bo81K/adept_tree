#pragma once

#include <QString>

#include "data/TreeNode.h"

class TreeFilter
{
public:
    bool isEmpty() const;

    bool matchesName(const QString &name) const;
    bool matchesValue(double value) const;
    bool matchesLeaf(const TreeNode &leaf) const;

    QString nameFilter;
    QString valueFilter;
    bool searchInNodeNames = false;
};
