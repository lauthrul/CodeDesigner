#include "MainWindow.h"
#include "core/DataManager.h"
#include "ui/GlobalVariablesDialog.h"
#include "ui/BinCodeDialog.h"
#include "ui/DecEditorDialog.h"
#include "ui/ToolPage.h"
#include <QMetaProperty>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QListView>
#include <QTextEdit>

struct MainWindow::Private
{
    ToolPage* m_templateToolPage = nullptr;
};

//////////////////////////////////////////////////////////////////////////

MainWindow::MainWindow(const QString& filePath, QWidget* parent)
    : QMainWindow(parent), d(new MainWindow::Private)
{
    ui.setupUi(this);
    initUI();

    if (filePath.isEmpty()) onNew();
    else onOpen(filePath);
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

    initTemplateToolBox();

    // connections
    connect(ui.actionNew, &QAction::triggered, this, &MainWindow::onNew);
    connect(ui.actionOpen, &QAction::triggered, this, [&]() { onOpen(); });
    connect(ui.actionSave, &QAction::triggered, this, &MainWindow::onSave);
    connect(ui.actionSaveAs, &QAction::triggered, this, &MainWindow::onSaveAs);
    connect(ui.actionPreviewDEC, &QAction::triggered, this, [&]() { previewCode(tr("dec")); });
    connect(ui.actionGenerateDEC, &QAction::triggered, this, [&]() { generateCode(tr("dec")); });
    connect(ui.actionPreviewCode, &QAction::triggered, this, [&]() { previewCode(tr("code")); });
    connect(ui.actionGenerateCode, &QAction::triggered, this, [&]() { generateCode(tr("code")); });
    connect(ui.actionDecEditor, &QAction::triggered, this, &MainWindow::onDecEditor);
    connect(ui.btnAddFunction, &QPushButton::clicked, this, &MainWindow::onAddFunction);
    connect(ui.btnDelFunction, &QPushButton::clicked, this, &MainWindow::onDelFunction);
    connect(DM_INST, &DataManager::nodeDoubleClicked, this, &MainWindow::onNodeDoubleClicked);
    connect(ui.actionGlobalVariables, &QAction::triggered, this, &MainWindow::onGlobalVariables);
    connect(ui.actionBinCodes, &QAction::triggered, this, &MainWindow::onBinCodes);
    connect(ui.actionAutoConnect, &QAction::triggered, this, [&]() { ui.flowView->autoConnect(); });
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

        onNodeDoubleClicked(uid, false);
    });
    traverseNodeInfo(&DM_INST->node(), nullptr, [&](NodeInfo * node, NodeInfo * parent, void* userData)
    {
        if (parent == nullptr || parent == &DM_INST->node())
        {
            if (node->type == NT_Function && node->function.type != FT_API)
                ui.cmbFunctions->addItem(node->function.name, node->uid);
        }
        return true;
    }, nullptr);
}

void MainWindow::initTemplateToolBox()
{
    NodeInfoList list;
    for (auto node : DM_INST->node().children)
    {
        if (node.type == NT_Function && node.function.type == FT_Custom)
            list.push_back(node);
    }
    ui.toolBox->removeToolPage(d->m_templateToolPage);
    d->m_templateToolPage = ui.toolBox->createListToolPage(tr("Custom Functions"), list, QListView::ListMode, true);
}

void MainWindow::onNew()
{
    auto file = DataManager::create();
    DM_INST->setPath("");
    DM_INST->setFile(file);
    ui.flowView->load(file.node);
    initNavigator();
    setWindowTitle(QString("%1 - %2").arg(tr("Code Designer")).arg("New File"));
}

void MainWindow::onOpen(const QString& filePath /*= ""*/)
{
    auto path(filePath);
    if (path.isEmpty())
    {
        QFileDialog dialog(this, tr("Open File"), "", "MetaATE Flow UI (*.mfu);;All Files (*.*)");
        dialog.setAcceptMode(QFileDialog::AcceptOpen);
        if (dialog.exec() == QDialog::Accepted)
            path = dialog.selectedFiles().first();
    }
    if (path.isEmpty()) return;

    File file;
    auto ret = DataManager::load(file, path);
    if (ret != 0)
    {
        QMessageBox::warning(this, tr("warning"), tr("load ui file fail: ") + QString::number(ret));
        return;
    }
    DM_INST->setPath(path);
    DM_INST->setFile(file);
    ui.flowView->load(file.node);
    initNavigator();
    initTemplateToolBox();
    setWindowTitle(QString("%1 - %2").arg(tr("Code Designer")).arg(path));
}

QString MainWindow::selectSaveFile(const QString& title, const QString& defaultName)
{
    QFileDialog dialog(this, title, defaultName, "MetaATE Flow UI (*.mfu);;All Files (*.*)");
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if (dialog.exec() == QDialog::Accepted)
        return dialog.selectedFiles().first();
    return "";
}

void MainWindow::onSave()
{
    if (DM_INST->path().isEmpty())
    {
        auto filePath = selectSaveFile(tr("Save"), "New File");
        if (filePath.isEmpty()) return;

        DM_INST->setPath(filePath);
    }
    DM_INST->save();
}

void MainWindow::onSaveAs()
{
    auto filePath = selectSaveFile(tr("Save As"), DM_INST->path());
    if (filePath.isEmpty()) return;

    DM_INST->setPath(filePath);
    DM_INST->save();
}

void MainWindow::previewCode(const QString& type)
{
    int err = 0;
    QString code;
    if (type == tr("dec"))
        code = DM_INST->generateDec(false, &err);
    else if (type == tr("code"))
        code = DM_INST->generateCode(false, &err);
    if (err != 0)
        QMessageBox::warning(this, tr("Warning"), QString(tr("Generate %1 fail: %2")).arg(type, code));
    else
    {
        auto edit = new QTextEdit();
        edit->setWindowTitle(QString(tr("Preview %1")).arg(type));
        edit->setText(code);
        edit->setReadOnly(true);
        edit->resize(size() * 0.8);
        edit->show();
    }
}

void MainWindow::generateCode(const QString& type)
{
    onSave();

    int err = 0;
    QString code;
    if (type == tr("dec"))
        code = DM_INST->generateDec(true, &err);
    else if (type == tr("code"))
        code = DM_INST->generateCode(true, &err);
    if (err != 0)
        QMessageBox::warning(this, tr("Warning"), QString(tr("Generate %1 fail: %2")).arg(type, code));
    else
        QMessageBox::information(this, tr("Information"), QString(tr("Generate %1 success!")).arg(type));
}

void MainWindow::onNodeDoubleClicked(const QString& uid, bool updateNavi)
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
    if (name.isEmpty()) return;

    if (DM_INST->node().findChildByName(name) != nullptr)
    {
        QMessageBox::information(this, tr("information"), tr("function [%1] is already exist!").arg(name));
        return;
    }

    NodeInfo node;
    node.icon = ":/images/icon_custom_function.png";
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
    initTemplateToolBox();
}

void MainWindow::onDelFunction()
{
    auto uid = ui.cmbFunctions->currentData(Qt::UserRole).toString();
    auto node = DM_INST->node().findChild(uid);
    if (node)
    {
        if (node->function.type != FT_Custom) return;

        DM_INST->node().removeChild(uid);
    }
    ui.cmbFunctions->removeItem(ui.cmbFunctions->currentIndex());
    initTemplateToolBox();
}

void MainWindow::onGlobalVariables()
{
    GlobalVariablesDialog dlg;
    dlg.exec();
}

void MainWindow::onBinCodes()
{
    BinCodeDialog dlg;
    dlg.exec();
}

void MainWindow::onDecEditor()
{
    DecEditorDialog dlg;
    dlg.exec();
}
