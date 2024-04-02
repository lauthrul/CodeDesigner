#include "MainWindow.h"
#include "core/DataManager.h"
#include <QMetaProperty>
#include <QInputDialog>
#include <QFileDialog>

MainWindow::MainWindow(const QString& filePath, QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    load(filePath);
    initUI();
}

MainWindow::~MainWindow()
{
}

void MainWindow::initUI()
{
    // tool box
    auto funcs = DataManager::loadFunctions("D:/CZTEK/MetaATE/ATE/sdk/include/TestPlan/TestPlan.h");
    ui.toolBox->createListToolPage(tr("DC functions"), funcs, true);

    // navigator
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
        ui.cmbFunctions->addItem(node->function.name, node->uid);
        return true;
    }, nullptr);
    connect(ui.btnAddFunction, &QPushButton::clicked, this, &MainWindow::onAddFunction);
    connect(ui.btnDelFunction, &QPushButton::clicked, this, &MainWindow::onDelFunction);

    // flow view
    ui.flowView->load(DM_INST->currentRootNode());

    // connections
    connect(ui.flowView, &FlowView::currentFlowNodeChanged,
            ui.propertyWdiget, &PropertyWidget::currentFlowNodeChanged);
    connect(DM_INST, &DataManager::nodeAdded, this, &MainWindow::onNodeAdded);
    connect(DM_INST, &DataManager::nodeSwitched, this, &MainWindow::onNodeSwitched);
    connect(ui.btnSave, &QPushButton::clicked, this, &MainWindow::onSave);
}

void MainWindow::load(const QString& filePath)
{
    NodeInfo root;
    if (filePath.isEmpty())
        root = DataManager::createNodeInfo();
    else
        root = DataManager::loadNodeInfo(filePath);
    DM_INST->setCurrentFilePath(filePath);
    DM_INST->setCurrentRootNode(root);
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

void MainWindow::onNodeAdded(QSharedPointer<NodeInfo> node)
{
    auto uid = ui.cmbFunctions->currentData(Qt::UserRole).toString();
    auto parent = DM_INST->currentRootNode().find(uid);
    if (parent) parent->add(*node);
}

void MainWindow::onSave()
{
    if (DM_INST->currentFilePath().isEmpty())
    {
        QFileDialog dialog(this, tr("Save File"), "", "MetaATE Flow UI File (*.mfu)");
        dialog.setAcceptMode(QFileDialog::AcceptSave);
        if (dialog.exec() == QDialog::Accepted)
        {
            QString fileName = dialog.selectedFiles().first();
            DM_INST->setCurrentFilePath(fileName);
            DM_INST->saveCurrentNodeInfo();
        }
    }
}
