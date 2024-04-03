#include "MainWindow.h"
#include "core/DataManager.h"
#include <QMetaProperty>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>

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
    auto funcs = DataManager::loadFunctions("D:/CZTEK/MetaATE/ATE/sdk/include/TestPlan/TestPlan.h");
    ui.toolBox->createListToolPage(tr("APIs"), funcs, true);

    // connections
    connect(ui.actionNew, &QAction::triggered, this, &MainWindow::onNew);
    connect(ui.actionOpen, &QAction::triggered, this, &MainWindow::onOpen);
    connect(ui.actionSave, &QAction::triggered, this, &MainWindow::onSave);
    connect(ui.btnAddFunction, &QPushButton::clicked, this, &MainWindow::onAddFunction);
    connect(ui.btnDelFunction, &QPushButton::clicked, this, &MainWindow::onDelFunction);
    connect(ui.flowView, &FlowView::nodeSelectionChanged, ui.propertyWdiget, &PropertyWidget::onNodeSelectionChanged);
    connect(DM_INST, &DataManager::nodeAdded, this, &MainWindow::onNodeAdded);
    connect(DM_INST, &DataManager::nodeSwitched, this, &MainWindow::onNodeSwitched);
    connect(DM_INST, &DataManager::connectionAdded, this, &MainWindow::onConnectionAdded);
    connect(DM_INST, &DataManager::nodePostionChanged, this, &MainWindow::onNodePostionChanged);
}

void MainWindow::initNavigator()
{
    ui.cmbFunctions->clear();
    connect(ui.cmbFunctions, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int index)
    {
        auto uid = ui.cmbFunctions->itemData(index, Qt::UserRole).toString();
        auto node = DM_INST->currentRootNode().find(uid);
        ui.btnDelFunction->setEnabled(node ? node->function.type == Function::Custom : false);

        onNodeSwitched(uid, false);
    });
    DataManager::traverseNodeInfo(&DM_INST->currentRootNode(), nullptr,
                                  [&](NodeInfo * node, NodeInfo * parent, void* userData)
    {
        if (node->function.type != Function::API)
            ui.cmbFunctions->addItem(node->function.name, node->uid);
        return true;
    }, nullptr);
}

void MainWindow::onNew()
{
    auto root = DataManager::createNodeInfo();
    DM_INST->setCurrentFilePath("");
    DM_INST->setCurrentRootNode(root);
    ui.flowView->load(root);
    initNavigator();
}

void MainWindow::onOpen()
{
    QFileDialog dialog(this, tr("Open File"), "", "MetaATE Flow UI (*.mfu);;All Files (*.*)");
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    if (dialog.exec() == QDialog::Accepted)
    {
        QString filePath = dialog.selectedFiles().first();
        NodeInfo root;
        auto ret = DataManager::loadNodeInfo(root, filePath);
        if (ret != 0)
        {
            QMessageBox::warning(this, tr("warning"), tr("load ui file fail: ") + QString::number(ret));
            return;
        }
        DM_INST->setCurrentFilePath(filePath);
        DM_INST->setCurrentRootNode(root);
        ui.flowView->load(root);
        initNavigator();
    }
}

void MainWindow::onSave()
{
    if (DM_INST->currentFilePath().isEmpty())
    {
        QFileDialog dialog(this, tr("Save File"), "", "MetaATE Flow UI (*.mfu);;All Files (*.*)");
        dialog.setAcceptMode(QFileDialog::AcceptSave);
        if (dialog.exec() == QDialog::Accepted)
        {
            QString filePath = dialog.selectedFiles().first();
            DM_INST->setCurrentFilePath(filePath);
        }
    }
    DM_INST->saveCurrentNodeInfo();
}

void MainWindow::onNodeSwitched(const QString& uid, bool updateNavi)
{
    auto node = DM_INST->currentRootNode().find(uid);
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
        node.function.type = Function::Custom;
        node.function.name = name;
        DM_INST->currentRootNode().add(node);

        ui.cmbFunctions->addItem(name, node.uid);
        ui.cmbFunctions->setCurrentText(name);
    }
}

void MainWindow::onDelFunction()
{
    auto uid = ui.cmbFunctions->currentData(Qt::UserRole).toString();
    auto node = DM_INST->currentRootNode().find(uid);
    if (node)
    {
        if (node->function.type != Function::Custom) return;

        DM_INST->currentRootNode().removeByUid(uid);
    }
    ui.cmbFunctions->removeItem(ui.cmbFunctions->currentIndex());
}

void MainWindow::onNodeAdded(const QString& uid, QSharedPointer<NodeInfo> node)
{
    auto parent = DM_INST->currentRootNode().find(uid);
    if (parent) parent->add(*node);
}

void MainWindow::onConnectionAdded(const QString& uid, const QString& connection)
{
    auto node = DM_INST->currentRootNode().find(uid);
    if (node) node->connections.append(connection);
}

void MainWindow::onNodePostionChanged(const QString& uid, const QPointF& pos)
{
    auto node = DM_INST->currentRootNode().find(uid);
    if (node) node->pos = pos;
}
