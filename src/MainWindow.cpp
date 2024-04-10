#include "MainWindow.h"
#include "core/DataManager.h"
#include "ui/GlobalVariablesDialog.h"
#include <QMetaProperty>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QListView>

MainWindow::MainWindow(const QString& filePath, QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    initUI();
}

MainWindow::~MainWindow()
{
}

void MainWindow::initUI()
{
    // tool box
    ui.toolBox->createListToolPage(tr("Process Controls"), DataManager::systemNodes(), QListView::IconMode, true);
    auto funcs = DataManager::loadFunctions("D:/CZTEK/MetaATE/ATE/sdk/include/TestPlan/TestPlan.h");
    NodeInfoList list;
    for (const auto& func : funcs)
    {
        NodeInfo node{ NT_Function, func.name, ":/images/icon_fx.png" };
        node.function = func;
        list.push_back(node);
    }
    ui.toolBox->createListToolPage(tr("APIs"), list, QListView::ListMode, true);

    // connections
    connect(ui.actionNew, &QAction::triggered, this, &MainWindow::onNew);
    connect(ui.actionOpen, &QAction::triggered, this, &MainWindow::onOpen);
    connect(ui.actionSave, &QAction::triggered, this, &MainWindow::onSave);
    connect(ui.btnAddFunction, &QPushButton::clicked, this, &MainWindow::onAddFunction);
    connect(ui.btnDelFunction, &QPushButton::clicked, this, &MainWindow::onDelFunction);
    connect(ui.flowView, &FlowView::nodeSelectionChanged, ui.propertyBrowser, &TreePropertyBrowser::onNodeSelectionChanged);
    connect(DM_INST, &DataManager::nodeAdded, this, &MainWindow::onNodeAdded);
    connect(DM_INST, &DataManager::nodeSwitched, this, &MainWindow::onNodeSwitched);
    connect(DM_INST, &DataManager::connectionAdded, this, &MainWindow::onConnectionAdded);
    connect(DM_INST, &DataManager::nodePostionChanged, this, &MainWindow::onNodePostionChanged);
    connect(ui.actionGlobalVariables, &QAction::triggered, this, &MainWindow::onGlobalVariables);
    connect(ui.actionBinCodes, &QAction::triggered, this, &MainWindow::onBinCodes);
    connect(ui.actionAlignLeft, &QAction::triggered, this, [&]() {ui.flowView->align(Left); });
    connect(ui.actionAlignRight, &QAction::triggered, this, [&]() {ui.flowView->align(Right); });
    connect(ui.actionAlignTop, &QAction::triggered, this, [&]() {ui.flowView->align(Top); });
    connect(ui.actionAlignBottom, &QAction::triggered, this, [&]() {ui.flowView->align(Bottom); });
    connect(ui.actionAlignHCenter, &QAction::triggered, this, [&]() {ui.flowView->align(HCenter); });
    connect(ui.actionAlignVCenter, &QAction::triggered, this, [&]() {ui.flowView->align(VCenter); });
}

void MainWindow::initNavigator()
{
    ui.cmbFunctions->clear();
    connect(ui.cmbFunctions, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int index)
    {
        auto uid = ui.cmbFunctions->itemData(index, Qt::UserRole).toString();
        auto node = DM_INST->node().findChild(uid);
        ui.btnDelFunction->setEnabled(node ? node->function.type == FT_Custom : false);

        onNodeSwitched(uid, false);
    });
    traverseNodeInfo(&DM_INST->node(), nullptr,
                     [&](NodeInfo * node, NodeInfo * parent, void* userData)
    {
        if (node->type == NT_Function && node->function.type != FT_API)
            ui.cmbFunctions->addItem(node->function.name, node->uid);
        return true;
    }, nullptr);
}

void MainWindow::onNew()
{
    auto file = DataManager::create();
    DM_INST->setPath("");
    DM_INST->setFile(file);
    ui.flowView->load(file.node);
    initNavigator();
}

void MainWindow::onOpen()
{
    QFileDialog dialog(this, tr("Open File"), "", "MetaATE Flow UI (*.mfu);;All Files (*.*)");
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    if (dialog.exec() == QDialog::Accepted)
    {
        QString filePath = dialog.selectedFiles().first();
        File file;
        auto ret = DataManager::load(file, filePath);
        if (ret != 0)
        {
            QMessageBox::warning(this, tr("warning"), tr("load ui file fail: ") + QString::number(ret));
            return;
        }
        DM_INST->setPath(filePath);
        DM_INST->setFile(file);
        ui.flowView->load(file.node);
        initNavigator();
    }
}

void MainWindow::onSave()
{
    if (DM_INST->path().isEmpty())
    {
        QFileDialog dialog(this, tr("Save File"), "", "MetaATE Flow UI (*.mfu);;All Files (*.*)");
        dialog.setAcceptMode(QFileDialog::AcceptSave);
        if (dialog.exec() == QDialog::Accepted)
        {
            QString filePath = dialog.selectedFiles().first();
            DM_INST->setPath(filePath);
        }
    }
    DM_INST->save();
}

void MainWindow::onNodeSwitched(const QString& uid, bool updateNavi)
{
    auto node = DM_INST->node().findChild(uid);
    if (node)
    {
        ui.flowView->load(*node);
        if (updateNavi)
            ui.cmbFunctions->setCurrentText(node->function.name);
    }
}

void MainWindow::onAddFunction()
{
    auto name = QInputDialog::getText(this, tr("Add Function"), tr("Function Name"));
    if (!name.isEmpty())
    {
        NodeInfo node;
        node.uid = DataManager::genUUid();
        node.name = name;
        node.type = NT_Function;
        node.function.type = FT_Custom;
        node.function.retType = "void";
        node.function.name = name;
        node.function.raw = QString("%1 %2()").arg(node.function.retType).arg(name);
        DM_INST->node().addChild(node);

        ui.cmbFunctions->addItem(name, node.uid);
        ui.cmbFunctions->setCurrentText(name);
    }
}

void MainWindow::onDelFunction()
{
    auto uid = ui.cmbFunctions->currentData(Qt::UserRole).toString();
    auto node = DM_INST->node().findChild(uid);
    if (node)
    {
        if (node->function.type != FT_Custom) return;

        DM_INST->node().removeChildByUid(uid);
    }
    ui.cmbFunctions->removeItem(ui.cmbFunctions->currentIndex());
}

void MainWindow::onNodeAdded(const QString& uid, QSharedPointer<NodeInfo> node)
{
    auto parent = DM_INST->node().findChild(uid);
    if (parent) parent->addChild(*node);
}

void MainWindow::onConnectionAdded(const QString& uid, const QString& connection)
{
    auto node = DM_INST->node().findChild(uid);
    if (node) node->connections.append(connection);
}

void MainWindow::onNodePostionChanged(const QString& uid, const QPointF& pos)
{
    auto node = DM_INST->node().findChild(uid);
    if (node) node->pos = pos;
}

void MainWindow::onGlobalVariables()
{
    GlobalVariablesDialog dlg;
    dlg.exec();
}

void MainWindow::onBinCodes()
{

}
