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
    void load(const QString& filePath);

protected slots:
    void onSwitchFunction(int index);
    void onAddFunction();
    void onDelFunction();
    void onNodeAdded(QSharedPointer<NodeInfo> node);

private:
    Ui::MainWindow ui;
};

