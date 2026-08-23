#pragma once

#include <QtGlobal>
#include <QString>

struct TreeNode
{
    bool isRoot() const;
    QString valueAsString() const;

    qint64 id = 0;
    qint64 parentId = 0;
    QString name;
    double value = 0.0;
    bool isLeaf = false;
};
