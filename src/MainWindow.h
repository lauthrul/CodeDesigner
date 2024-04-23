#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(const QString& filePath, QWidget* parent = nullptr);
    ~MainWindow();

public:
    void initUI();
    void initNavigator();
    void initTemplateToolBox();

protected slots:
    void onNew();
    void onOpen(const QString& filePath = "");
    void onSave();
    void onSaveAs();
    void previewCode(const QString& type);
    void generateCode(const QString& type);
    void onNodeDoubleClicked(const QString& uid, bool updateNavi);
    void onAddFunction();
    void onEditFunction();
    void onDelFunction();
    void onGlobalVariables();
    void onBinCodes();
    void onDecEditor();

private:
    QString selectSaveFile(const QString& title, const QString& defaultName);

private:
    struct Private;
    QScopedPointer<Private> d;
    Ui::MainWindow ui;
};

