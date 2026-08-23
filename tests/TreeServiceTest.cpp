#include <QtTest>

#include <QSharedPointer>
#include <QTemporaryFile>

#include "TestDatabaseUtility.h"
#include "data/TreeNode.h"
#include "service/TreeExporter.h"
#include "service/TreeService.h"

class TreeServiceTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();

    void testAddNodeAndLeaf();
    void testUpdateLeaf();
    void testUpdateName();
    void testRemoveLeaf();
    void testRemoveNodeWithLeaves();
    void testStatistics();
    void testNameFilterCanSearchInNodeNames();
    void testValueFilterShowsOnlyMatchingLeaves();
    void testExportAndImport();

private:
    QSharedPointer<TreeRepository> repository;
    QSharedPointer<TreeExporter> exporter;
    QSharedPointer<TreeService> service;
};

void TreeServiceTest::initTestCase()
{
    repository = TestDatabaseUtility::createTestRepository();

    if (repository.isNull())
    {
        QSKIP("PostgreSQL test database is not available");
    }

    exporter = QSharedPointer<TreeExporter>::create();
    service = QSharedPointer<TreeService>::create(repository, exporter);

    QVERIFY(service->reload());
}

void TreeServiceTest::init()
{
    QVERIFY(repository->clearAll());
    QVERIFY(service->reload());
}

void TreeServiceTest::testAddNodeAndLeaf()
{
    const qint64 rootId = service->addNode(0, QStringLiteral("Root"));

    QVERIFY(rootId > 0);

    const qint64 leafId = service->addLeaf(rootId, QStringLiteral("Leaf"), 3.25);

    QVERIFY(leafId > 0);

    const QVector<TreeNode> nodes = service->nodes();

    QCOMPARE(static_cast<int>(nodes.size()), 2);
}

void TreeServiceTest::testUpdateLeaf()
{
    const qint64 rootId = service->addNode(0, QStringLiteral("Root"));

    QVERIFY(rootId > 0);

    const qint64 leafId = service->addLeaf(rootId, QStringLiteral("Leaf"), 1.25);

    QVERIFY(leafId > 0);

    QVERIFY(!service->updateLeaf(rootId, QStringLiteral("Bad update"), 5.0));

    QVERIFY(service->updateLeaf(leafId, QStringLiteral("Updated leaf"), 8.5));

    const TreeNode updatedLeaf = service->findNode(leafId);

    QCOMPARE(updatedLeaf.name, QStringLiteral("Updated leaf"));
    QCOMPARE(updatedLeaf.value, 8.5);
}

void TreeServiceTest::testUpdateName()
{
    const qint64 rootId = service->addNode(0, QStringLiteral("Root"));

    QVERIFY(rootId > 0);

    const qint64 leafId = service->addLeaf(rootId, QStringLiteral("Leaf"), 2.5);

    QVERIFY(leafId > 0);

    QVERIFY(!service->updateName(rootId, QStringLiteral("   ")));

    QVERIFY(service->updateName(rootId, QStringLiteral("New root")));
    QVERIFY(service->updateName(leafId, QStringLiteral("New leaf")));

    const TreeNode updatedRoot = service->findNode(rootId);
    const TreeNode updatedLeaf = service->findNode(leafId);

    QCOMPARE(updatedRoot.name, QStringLiteral("New root"));
    QCOMPARE(updatedLeaf.name, QStringLiteral("New leaf"));
}

void TreeServiceTest::testRemoveLeaf()
{
    const qint64 rootId = service->addNode(0, QStringLiteral("Root"));

    QVERIFY(rootId > 0);

    const qint64 leafId = service->addLeaf(rootId, QStringLiteral("Leaf"), 6.5);

    QVERIFY(leafId > 0);

    QVERIFY(!service->removeLeaf(rootId));
    QVERIFY(service->removeLeaf(leafId));

    const TreeStatistics statistics = service->statistics();

    QCOMPARE(statistics.nodeCount, 1);
    QCOMPARE(statistics.leafCount, 0);
}

void TreeServiceTest::testRemoveNodeWithLeaves()
{
    const qint64 rootId = service->addNode(0, QStringLiteral("Root"));

    QVERIFY(rootId > 0);

    const qint64 leafId = service->addLeaf(rootId, QStringLiteral("Leaf"), 7.5);

    QVERIFY(leafId > 0);

    QVERIFY(!service->removeNodeWithLeaves(leafId));
    QVERIFY(service->removeNodeWithLeaves(rootId));

    QCOMPARE(static_cast<int>(service->nodes().size()), 0);
}

void TreeServiceTest::testStatistics()
{
    const qint64 rootId = service->addNode(0, QStringLiteral("Root"));

    QVERIFY(rootId > 0);

    const qint64 firstLeafId = service->addLeaf(rootId, QStringLiteral("First leaf"), 1.25);
    const qint64 secondLeafId = service->addLeaf(rootId, QStringLiteral("Second leaf"), 7.75);

    QVERIFY(firstLeafId > 0);
    QVERIFY(secondLeafId > 0);

    const TreeStatistics statistics = service->statistics();

    QCOMPARE(statistics.nodeCount, 1);
    QCOMPARE(statistics.leafCount, 2);
    QCOMPARE(statistics.hasLeafValues, true);
    QCOMPARE(statistics.maxValue, 7.75);
    QCOMPARE(statistics.minValue, 1.25);
}

void TreeServiceTest::testNameFilterCanSearchInNodeNames()
{
    const qint64 rootId = service->addNode(0, QStringLiteral("Alpha Root"));

    QVERIFY(rootId > 0);

    const qint64 branchId = service->addNode(rootId, QStringLiteral("Beta Branch"));

    QVERIFY(branchId > 0);

    const qint64 leafId = service->addLeaf(branchId, QStringLiteral("Gamma Leaf"), 1.0);

    QVERIFY(leafId > 0);

    TreeFilter filter;

    filter.nameFilter = QStringLiteral("beta");
    filter.searchInNodeNames = false;

    QVector<TreeNode> visibleNodes = service->visibleNodes(filter);

    QCOMPARE(static_cast<int>(visibleNodes.size()), 0);

    filter.searchInNodeNames = true;

    visibleNodes = service->visibleNodes(filter);

    bool branchVisible = false;
    bool leafVisible = false;

    for (const TreeNode &node : visibleNodes)
    {
        if (node.id == branchId)
        {
            branchVisible = true;
        }

        if (node.id == leafId)
        {
            leafVisible = true;
        }
    }

    QVERIFY(branchVisible);
    QVERIFY(leafVisible);
}

void TreeServiceTest::testValueFilterShowsOnlyMatchingLeaves()
{
    const qint64 rootId = service->addNode(0, QStringLiteral("Root"));

    QVERIFY(rootId > 0);

    const qint64 firstLeafId = service->addLeaf(rootId, QStringLiteral("First"), 10.5);
    const qint64 secondLeafId = service->addLeaf(rootId, QStringLiteral("Second"), 20.25);

    QVERIFY(firstLeafId > 0);
    QVERIFY(secondLeafId > 0);

    TreeFilter filter;

    filter.valueFilter = QStringLiteral("20.2");

    const QVector<TreeNode> visibleNodes = service->visibleNodes(filter);

    bool firstLeafVisible = false;
    bool secondLeafVisible = false;

    for (const TreeNode &node : visibleNodes)
    {
        if (node.id == firstLeafId)
        {
            firstLeafVisible = true;
        }

        if (node.id == secondLeafId)
        {
            secondLeafVisible = true;
        }
    }

    QVERIFY(!firstLeafVisible);
    QVERIFY(secondLeafVisible);
}

void TreeServiceTest::testExportAndImport()
{
    const qint64 rootId = service->addNode(0, QStringLiteral("Export root"));

    QVERIFY(rootId > 0);

    const qint64 leafId = service->addLeaf(rootId, QStringLiteral("Export leaf"), 42.5);

    QVERIFY(leafId > 0);

    QTemporaryFile temporaryFile;

    QVERIFY(temporaryFile.open());

    const QString filePath = temporaryFile.fileName();

    temporaryFile.close();

    QVERIFY(service->exportToFile(filePath));

    QVERIFY(repository->clearAll());
    QVERIFY(service->reload());

    QCOMPARE(static_cast<int>(service->nodes().size()), 0);

    QVERIFY(service->loadFromFile(filePath));

    const TreeStatistics statistics = service->statistics();

    QCOMPARE(statistics.nodeCount, 1);
    QCOMPARE(statistics.leafCount, 1);
    QCOMPARE(statistics.maxValue, 42.5);
    QCOMPARE(statistics.minValue, 42.5);
}

QTEST_GUILESS_MAIN(TreeServiceTest)
#include "TreeServiceTest.moc"
