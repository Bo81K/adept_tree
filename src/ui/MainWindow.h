#pragma once

#include <QMainWindow>
#include <QScopedPointer>
#include <QSharedPointer>

#include "data/TreeRepository.h"
#include "service/TreeExporter.h"
#include "service/TreeService.h"

class FilterWidget;
class StatusWidget;
class TreeWidget;

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void slotAddNode();
    void slotAddLeaf();
    void slotRenameSelectedItem();
    void slotDeleteLeaf();
    void slotDeleteNode();
    void slotExport();
    void slotImport();
    void slotRefresh();
    void slotFilterChanged();
    void slotItemEdited(qint64 nodeId, int column, const QString &newValue);

private:
    void initializeDatabase();
    void initializeUi();
    void initializeConnections();

    void refreshTree();
    void refreshStatus();
    void showErrorMessage(const QString &message);

    qint64 selectedParentIdForNewItem() const;

    QScopedPointer<Ui::MainWindow> userInterface;

    QSharedPointer<TreeRepository> treeRepository;
    QSharedPointer<TreeExporter> treeExporter;
    QSharedPointer<TreeService> treeService;

    TreeWidget *treeWidget = nullptr;
    FilterWidget *filterWidget = nullptr;
    StatusWidget *statusWidget = nullptr;
};
