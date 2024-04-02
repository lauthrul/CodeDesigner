#include "MainWindow.h"
#include "core/DataManager.h"
#include <QMetaProperty>
#include <QInputDialog>
#include <QDebug>

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
    auto funcs = DataManager::loadFunctionsFromFile("D:/CZTEK/MetaATE/ATE/sdk/include/TestPlan/TestPlan.h");
    ui.toolBox->createListToolPage(tr("DC functions"), funcs, true);

    // navigator
    connect(ui.cmbFunctions, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onSwitchFunction);
    DataManager::traverseNodeInfo(&DataManager::instance()->rootNode(), nullptr,
                                  [&](NodeInfo * node, NodeInfo * parent, void* userData)
    {
        ui.cmbFunctions->addItem(node->function.name, node->uid);
        return true;
    }, nullptr);
    connect(ui.btnAddFunction, &QPushButton::clicked, this, &MainWindow::onAddFunction);
    connect(ui.btnDelFunction, &QPushButton::clicked, this, &MainWindow::onDelFunction);

    // flow view
    ui.flowView->load(DataManager::instance()->rootNode());

    // connections
    connect(ui.flowView, &FlowView::currentFlowNodeChanged,
            ui.propertyWdiget, &PropertyWidget::currentFlowNodeChanged);
    connect(DataManager::instance(), &DataManager::nodeAdded, this, &MainWindow::onNodeAdded);
}

void MainWindow::load(const QString& filePath)
{
    NodeInfo root;
    if (filePath.isEmpty())
        root = DataManager::create();
    else
        root = DataManager::load(filePath);
    DataManager::instance()->setFilePath(filePath);
    DataManager::instance()->setRootNode(root);
}

void MainWindow::onSwitchFunction(int index)
{
    auto funcName = ui.cmbFunctions->itemText(index);
    ui.btnDelFunction->setDisabled(DataManager::isSystemFunction(funcName));

    auto uid = ui.cmbFunctions->itemData(index, Qt::UserRole).toString();
    auto node = DataManager::instance()->rootNode().find(uid);
    if (node) ui.flowView->load(*node);
}

void MainWindow::onAddFunction()
{
    auto name = QInputDialog::getText(this, tr("Add Function"), tr("Function Name"));
    if (!name.isEmpty())
    {
        NodeInfo node;
        node.uid = DataManager::genUUid();
        node.function.name = name;
        DataManager::instance()->rootNode().add(node);

        ui.cmbFunctions->addItem(name, node.uid);
        ui.cmbFunctions->setCurrentText(name);
    }
}

void MainWindow::onDelFunction()
{
    auto text = ui.cmbFunctions->currentText();
    if (DataManager::isSystemFunction(text)) return;

    DataManager::instance()->rootNode().removeByName(text);
    ui.cmbFunctions->removeItem(ui.cmbFunctions->currentIndex());
}

void MainWindow::onNodeAdded(QSharedPointer<NodeInfo> node)
{
    auto uid = ui.cmbFunctions->currentData(Qt::UserRole).toString();
    auto parent = DataManager::instance()->rootNode().find(uid);
    if (parent) parent->add(*node);
}
