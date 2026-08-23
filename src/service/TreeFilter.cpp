#include "service/TreeFilter.h"

bool TreeFilter::isEmpty() const
{
    return nameFilter.trimmed().isEmpty()
    && valueFilter.trimmed().isEmpty();
}

bool TreeFilter::matchesName(const QString &name) const
{
    if (nameFilter.trimmed().isEmpty())
    {
        return true;
    }

    return name.contains(nameFilter.trimmed(), Qt::CaseInsensitive);
}

bool TreeFilter::matchesValue(double value) const
{
    if (valueFilter.trimmed().isEmpty())
    {
        return true;
    }

    const QString valueText = QString::number(value, 'g', 12);

    return valueText.contains(valueFilter.trimmed(), Qt::CaseInsensitive);
}

bool TreeFilter::matchesLeaf(const TreeNode &leaf) const
{
    if (!leaf.isLeaf)
    {
        return false;
    }

    return matchesName(leaf.name) && matchesValue(leaf.value);
}
