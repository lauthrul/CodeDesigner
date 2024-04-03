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

protected slots:
    void onNew();
    void onOpen();
    void onSave();
    void onNodeSwitched(const QString& uid, bool updateNavi);
    void onAddFunction();
    void onDelFunction();
    void onNodeAdded(const QString& uid, QSharedPointer<NodeInfo> node);
    void onConnectionAdded(const QString& uid, const QString& connection);
    void onNodePostionChanged(const QString& uid, const QPointF& pos);

private:
    Ui::MainWindow ui;
};

