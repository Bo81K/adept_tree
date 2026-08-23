#include "ui/MainWindow.h"
#include "ui_MainWindow.h"
#include "ui/FilterWidget.h"
#include "ui/StatusWidget.h"
#include "ui/TreeWidget.h"

#include <QAction>
#include <QFileDialog>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QtGlobal>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , userInterface(new Ui::MainWindow)
    , treeRepository(QSharedPointer<TreeRepository>::create())
    , treeExporter(QSharedPointer<TreeExporter>::create())
    , treeService(QSharedPointer<TreeService>::create(treeRepository, treeExporter))
{
    userInterface->setupUi(this);

    setWindowTitle(QStringLiteral("Адепт - Дерево"));

    initializeUi();
    initializeDatabase();
    initializeConnections();

    refreshTree();
    refreshStatus();
}

MainWindow::~MainWindow() = default;

void MainWindow::slotAddNode()
{
    bool nameAccepted = false;

    const QString nodeName = QInputDialog::getText(this,
                                                   QStringLiteral("Добавление узла"),
                                                   QStringLiteral("Название узла"),
                                                   QLineEdit::Normal,
                                                   QString(),
                                                   &nameAccepted);

    if (!nameAccepted || nodeName.trimmed().isEmpty())
    {
        return;
    }

    const qint64 parentId = selectedParentIdForNewItem();
    const qint64 nodeId = treeService->addNode(parentId, nodeName.trimmed());

    if (nodeId == 0)
    {
        showErrorMessage(treeService->lastError());
        return;
    }

    refreshTree();
    refreshStatus();
}

void MainWindow::slotAddLeaf()
{
    bool nameAccepted = false;

    const QString leafName = QInputDialog::getText(this,
                                                   QStringLiteral("Добавление листа"),
                                                   QStringLiteral("Название листа"),
                                                   QLineEdit::Normal,
                                                   QString(),
                                                   &nameAccepted);

    if (!nameAccepted || leafName.trimmed().isEmpty())
    {
        return;
    }

    bool valueAccepted = false;

    const double leafValue = QInputDialog::getDouble(this,
                                                     QStringLiteral("Добавление листа"),
                                                     QStringLiteral("Значение листа"),
                                                     0.0,
                                                     -1000000000.0,
                                                     1000000000.0,
                                                     6,
                                                     &valueAccepted);

    if (!valueAccepted)
    {
        return;
    }

    const qint64 parentId = selectedParentIdForNewItem();
    const qint64 leafId = treeService->addLeaf(parentId, leafName.trimmed(), leafValue);

    if (leafId == 0)
    {
        showErrorMessage(treeService->lastError());
        return;
    }

    refreshTree();
    refreshStatus();
}

void MainWindow::slotRenameSelectedItem()
{
    const qint64 selectedNodeId = treeWidget->selectedNodeId();
    const TreeNode selectedNode = treeService->findNode(selectedNodeId);

    if (selectedNode.id == 0)
    {
        showErrorMessage(QStringLiteral("Выберите узел или лист для переименования"));
        return;
    }

    treeWidget->editSelectedName();
}

void MainWindow::slotDeleteLeaf()
{
    const qint64 selectedNodeId = treeWidget->selectedNodeId();
    const TreeNode leafNode = treeService->findNode(selectedNodeId);

    if (leafNode.id == 0 || !leafNode.isLeaf)
    {
        showErrorMessage(QStringLiteral("Выберите лист для удаления"));
        return;
    }

    const QString question = QStringLiteral("Удалить лист \"%1\"?").arg(leafNode.name);

    if (QMessageBox::question(this, QStringLiteral("Удаление листа"), question) != QMessageBox::Yes)
    {
        return;
    }

    if (!treeService->removeLeaf(leafNode.id))
    {
        showErrorMessage(treeService->lastError());
        return;
    }

    refreshTree();
    refreshStatus();
}

void MainWindow::slotDeleteNode()
{
    const qint64 selectedNodeId = treeWidget->selectedNodeId();
    const TreeNode selectedNode = treeService->findNode(selectedNodeId);

    if (selectedNode.id == 0 || selectedNode.isLeaf)
    {
        showErrorMessage(QStringLiteral("Выберите узел для удаления"));
        return;
    }

    const QString question = QStringLiteral("Удалить узел \"%1\" со всеми листьями?").arg(selectedNode.name);

    if (QMessageBox::question(this, QStringLiteral("Удаление узла"), question) != QMessageBox::Yes)
    {
        return;
    }

    if (!treeService->removeNodeWithLeaves(selectedNode.id))
    {
        showErrorMessage(treeService->lastError());
        return;
    }

    refreshTree();
    refreshStatus();
}

void MainWindow::slotExport()
{
    const QString filter = QStringLiteral(
        "JSON файлы (*.json);;Текстовые файлы (*.txt);;Все файлы (*)");

    const QString filePath = QFileDialog::getSaveFileName(this,
                                                          QStringLiteral("Экспорт дерева"),
                                                          QString(),
                                                          filter,
                                                          nullptr);

    if (filePath.isEmpty())
    {
        return;
    }

    if (!treeService->exportToFile(filePath))
    {
        showErrorMessage(treeService->lastError());
        return;
    }

    statusBar()->showMessage(QStringLiteral("Дерево экспортировано"), 3000);
}

void MainWindow::slotImport()
{
    const QString filter = QStringLiteral(
        "JSON файлы (*.json);;Текстовые файлы (*.txt);;Все файлы (*)");

    const QString filePath = QFileDialog::getOpenFileName(this,
                                                          QStringLiteral("Импорт дерева"),
                                                          QString(),
                                                          filter);

    if (filePath.isEmpty())
    {
        return;
    }

    const QString question = QStringLiteral(
        "Текущее дерево будет заменено импортированными данными. Продолжить?");

    if (QMessageBox::question(this, QStringLiteral("Импорт дерева"), question) != QMessageBox::Yes)
    {
        return;
    }

    if (!treeService->loadFromFile(filePath))
    {
        showErrorMessage(treeService->lastError());
        return;
    }

    refreshTree();
    refreshStatus();

    statusBar()->showMessage(QStringLiteral("Дерево импортировано"), 3000);
}

void MainWindow::slotRefresh()
{
    if (!treeService->reload())
    {
        showErrorMessage(treeService->lastError());
        return;
    }

    refreshTree();
    refreshStatus();
}

void MainWindow::slotFilterChanged()
{
    refreshTree();
}

void MainWindow::slotItemEdited(qint64 nodeId, int column, const QString &newValue)
{
    const TreeNode node = treeService->findNode(nodeId);

    if (node.id == 0)
    {
        refreshTree();
        return;
    }

    if (column == 0)
    {
        if (!treeService->updateName(nodeId, newValue))
        {
            showErrorMessage(treeService->lastError());
            refreshTree();
            return;
        }
    }
    else if (column == 1)
    {
        if (!node.isLeaf)
        {
            refreshTree();
            return;
        }

        bool converted = false;

        const double value = newValue.toDouble(&converted);

        if (!converted)
        {
            showErrorMessage(QStringLiteral("Некорректное числовое значение"));
            refreshTree();
            return;
        }

        if (!treeService->updateLeaf(nodeId, node.name, value))
        {
            showErrorMessage(treeService->lastError());
            refreshTree();
            return;
        }
    }

    refreshTree();
    refreshStatus();
}

void MainWindow::initializeDatabase()
{
    const QString databaseName = qEnvironmentVariable("ADEPT_DB_NAME",
                                                      QStringLiteral("adept_tree"));
    const QString hostName = qEnvironmentVariable("ADEPT_DB_HOST",
                                                  QStringLiteral("localhost"));
    const QString userName = qEnvironmentVariable("ADEPT_DB_USER",
                                                  QStringLiteral("adept_user"));
    const QString password = qEnvironmentVariable("ADEPT_DB_PASSWORD",
                                                  QStringLiteral("adept_pass"));

    bool portConverted = false;

    int port = qEnvironmentVariable("ADEPT_DB_PORT",
                                    QStringLiteral("5432")).toInt(&portConverted);

    if (!portConverted)
    {
        port = 5432;
    }

    if (!treeRepository->open(databaseName, hostName, port, userName, password))
    {
        showErrorMessage(treeRepository->lastError());
        return;
    }

    if (!treeRepository->ensureSchema())
    {
        showErrorMessage(treeRepository->lastError());
        return;
    }

    treeService->reload();
}

void MainWindow::initializeUi()
{
    QVBoxLayout *centralLayout = new QVBoxLayout(userInterface->centralWidget);

    filterWidget = new FilterWidget(this);
    treeWidget = new TreeWidget(this);
    statusWidget = new StatusWidget(this);

    centralLayout->addWidget(filterWidget);
    centralLayout->addWidget(treeWidget, 1);

    statusBar()->addPermanentWidget(statusWidget, 1);

    QToolBar *mainToolBar = addToolBar(QStringLiteral("Главная панель"));

    mainToolBar->addAction(QStringLiteral("Добавить узел"), this, &MainWindow::slotAddNode);
    mainToolBar->addAction(QStringLiteral("Добавить лист"), this, &MainWindow::slotAddLeaf);
    mainToolBar->addAction(QStringLiteral("Переименовать элемент"), this, &MainWindow::slotRenameSelectedItem);
    mainToolBar->addAction(QStringLiteral("Удалить лист"), this, &MainWindow::slotDeleteLeaf);
    mainToolBar->addAction(QStringLiteral("Удалить узел"), this, &MainWindow::slotDeleteNode);
    mainToolBar->addSeparator();
    mainToolBar->addAction(QStringLiteral("Экспорт"), this, &MainWindow::slotExport);
    mainToolBar->addAction(QStringLiteral("Импорт"), this, &MainWindow::slotImport);
    mainToolBar->addAction(QStringLiteral("Обновить"), this, &MainWindow::slotRefresh);
    mainToolBar->setMovable(false);
}

void MainWindow::initializeConnections()
{
    connect(filterWidget,
            &FilterWidget::signalFilterChanged,
            this,
            &MainWindow::slotFilterChanged);

    connect(treeWidget,
            &TreeWidget::signalItemEdited,
            this,
            &MainWindow::slotItemEdited);
}

void MainWindow::refreshTree()
{
    TreeFilter filter;

    filter.nameFilter = filterWidget->nameFilter();
    filter.valueFilter = filterWidget->valueFilter();
    filter.searchInNodeNames = filterWidget->searchInNodeNames();

    treeWidget->setTreeData(treeService->visibleNodes(filter));
}

void MainWindow::refreshStatus()
{
    statusWidget->setStatistics(treeService->statistics());
}

void MainWindow::showErrorMessage(const QString &message)
{
    const QString displayMessage = message.isEmpty()
    ? QStringLiteral("Неизвестная ошибка")
    : message;

    QMessageBox::critical(this, QStringLiteral("Ошибка"), displayMessage);
}

qint64 MainWindow::selectedParentIdForNewItem() const
{
    const qint64 selectedNodeId = treeWidget->selectedNodeId();

    if (selectedNodeId == 0)
    {
        return 0;
    }

    const TreeNode selectedNode = treeService->findNode(selectedNodeId);

    if (selectedNode.id == 0)
    {
        return 0;
    }

    if (selectedNode.isLeaf)
    {
        return selectedNode.parentId;
    }

    return selectedNode.id;
}
