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
    void onNodeDoubleClicked(const QString& uid, bool updateNavi);
    void onAddFunction();
    void onDelFunction();
    void onGlobalVariables();
    void onBinCodes();

private:
    struct Private;
    QScopedPointer<Private> d;
    Ui::MainWindow ui;
};

