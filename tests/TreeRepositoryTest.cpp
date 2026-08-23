#include <QtTest>

#include "TestDatabaseUtility.h"
#include "data/TreeNode.h"

class TreeRepositoryTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();

    void testInsertAndFetch();
    void testUpdateNode();
    void testRemoveLeaf();
    void testRemoveNodeWithChildren();

private:
    QSharedPointer<TreeRepository> repository;
};

void TreeRepositoryTest::initTestCase()
{
    repository = TestDatabaseUtility::createTestRepository();

    if (repository.isNull())
    {
        QSKIP("PostgreSQL test database is not available");
    }
}

void TreeRepositoryTest::init()
{
    QVERIFY(repository->clearAll());
}

void TreeRepositoryTest::testInsertAndFetch()
{
    TreeNode rootNode;

    rootNode.name = QStringLiteral("Root");
    rootNode.isLeaf = false;

    const qint64 rootId = repository->insertNode(rootNode);

    QVERIFY(rootId > 0);

    TreeNode leafNode;

    leafNode.parentId = rootId;
    leafNode.name = QStringLiteral("Leaf");
    leafNode.value = 1.5;
    leafNode.isLeaf = true;

    const qint64 leafId = repository->insertNode(leafNode);

    QVERIFY(leafId > 0);

    const QVector<TreeNode> nodes = repository->fetchAllNodes();

    QCOMPARE(static_cast<int>(nodes.size()), 2);
    QCOMPARE(nodes.at(0).name, rootNode.name);
    QCOMPARE(nodes.at(1).parentId, rootId);
    QCOMPARE(nodes.at(1).name, leafNode.name);
    QCOMPARE(nodes.at(1).value, 1.5);
    QCOMPARE(nodes.at(1).isLeaf, true);
}

void TreeRepositoryTest::testUpdateNode()
{
    TreeNode leafNode;

    leafNode.name = QStringLiteral("Leaf");
    leafNode.value = 1.5;
    leafNode.isLeaf = true;

    const qint64 leafId = repository->insertNode(leafNode);

    QVERIFY(leafId > 0);

    TreeNode updatedNode;

    updatedNode.id = leafId;
    updatedNode.parentId = 0;
    updatedNode.name = QStringLiteral("Updated leaf");
    updatedNode.value = 2.5;
    updatedNode.isLeaf = true;

    QVERIFY(repository->updateNode(updatedNode));

    const QVector<TreeNode> nodes = repository->fetchAllNodes();

    QCOMPARE(static_cast<int>(nodes.size()), 1);
    QCOMPARE(nodes.at(0).name, updatedNode.name);
    QCOMPARE(nodes.at(0).value, updatedNode.value);
}

void TreeRepositoryTest::testRemoveLeaf()
{
    TreeNode rootNode;

    rootNode.name = QStringLiteral("Root");
    rootNode.isLeaf = false;

    const qint64 rootId = repository->insertNode(rootNode);

    QVERIFY(rootId > 0);

    TreeNode leafNode;

    leafNode.parentId = rootId;
    leafNode.name = QStringLiteral("Leaf");
    leafNode.value = 3.5;
    leafNode.isLeaf = true;

    const qint64 leafId = repository->insertNode(leafNode);

    QVERIFY(leafId > 0);
    QVERIFY(repository->removeNode(leafId));

    const QVector<TreeNode> nodes = repository->fetchAllNodes();

    QCOMPARE(static_cast<int>(nodes.size()), 1);
    QCOMPARE(nodes.at(0).id, rootId);
}

void TreeRepositoryTest::testRemoveNodeWithChildren()
{
    TreeNode rootNode;

    rootNode.name = QStringLiteral("Root");
    rootNode.isLeaf = false;

    const qint64 rootId = repository->insertNode(rootNode);

    QVERIFY(rootId > 0);

    TreeNode branchNode;

    branchNode.parentId = rootId;
    branchNode.name = QStringLiteral("Branch");
    branchNode.isLeaf = false;

    const qint64 branchId = repository->insertNode(branchNode);

    QVERIFY(branchId > 0);

    TreeNode leafNode;

    leafNode.parentId = branchId;
    leafNode.name = QStringLiteral("Leaf");
    leafNode.value = 4.5;
    leafNode.isLeaf = true;

    const qint64 leafId = repository->insertNode(leafNode);

    QVERIFY(leafId > 0);
    QVERIFY(repository->removeNode(rootId));

    const QVector<TreeNode> nodes = repository->fetchAllNodes();

    QCOMPARE(static_cast<int>(nodes.size()), 0);
}

QTEST_GUILESS_MAIN(TreeRepositoryTest)
#include "TreeRepositoryTest.moc"
