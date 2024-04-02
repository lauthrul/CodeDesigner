#include <QtWidgets/QApplication>
#include <QTranslator>
#include "MainWindow.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    QTranslator trans;
    trans.load(":/langs/zh_CN.qm");
    a.installTranslator(&trans);

    QString filePath;
    if (argc >= 2)
        filePath = argv[1];
    MainWindow w(filePath);
    w.show();
    return a.exec();
}