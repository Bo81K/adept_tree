#include <QtTest>

#include "data/TreeNode.h"
#include "service/TreeFilter.h"

class TreeFilterTest : public QObject
{
    Q_OBJECT

private slots:
    void testEmptyFilterMatchesLeaf();
    void testNameFilterIsCaseInsensitive();
    void testNameFilterRejectsMissingText();
    void testValueFilterMatchesStringRepresentation();
    void testValueFilterRejectsMissingText();
    void testNodeDoesNotMatch();

private:
    TreeNode createLeaf(const QString &name, double value) const;
};

TreeNode TreeFilterTest::createLeaf(const QString &name, double value) const
{
    TreeNode leaf;

    leaf.name = name;
    leaf.value = value;
    leaf.isLeaf = true;

    return leaf;
}

void TreeFilterTest::testEmptyFilterMatchesLeaf()
{
    const TreeNode leaf = createLeaf(QStringLiteral("Leaf"), 1.0);
    const TreeFilter filter;

    QVERIFY(filter.isEmpty());
    QVERIFY(filter.matchesLeaf(leaf));
}

void TreeFilterTest::testNameFilterIsCaseInsensitive()
{
    const TreeNode leaf = createLeaf(QStringLiteral("Important item"), 1.0);

    TreeFilter filter;
    filter.nameFilter = QStringLiteral("important");

    QVERIFY(filter.matchesLeaf(leaf));
}

void TreeFilterTest::testNameFilterRejectsMissingText()
{
    const TreeNode leaf = createLeaf(QStringLiteral("Important item"), 1.0);

    TreeFilter filter;
    filter.nameFilter = QStringLiteral("missing");

    QVERIFY(!filter.matchesLeaf(leaf));
}

void TreeFilterTest::testValueFilterMatchesStringRepresentation()
{
    const TreeNode leaf = createLeaf(QStringLiteral("Leaf"), 12.34);

    TreeFilter filter;
    filter.valueFilter = QStringLiteral("12.3");

    QVERIFY(filter.matchesLeaf(leaf));
}

void TreeFilterTest::testValueFilterRejectsMissingText()
{
    const TreeNode leaf = createLeaf(QStringLiteral("Leaf"), 12.34);

    TreeFilter filter;
    filter.valueFilter = QStringLiteral("99");

    QVERIFY(!filter.matchesLeaf(leaf));
}

void TreeFilterTest::testNodeDoesNotMatch()
{
    TreeNode node;

    node.name = QStringLiteral("Node");
    node.isLeaf = false;

    TreeFilter filter;

    QVERIFY(!filter.matchesLeaf(node));
}

QTEST_GUILESS_MAIN(TreeFilterTest)
#include "TreeFilterTest.moc"
