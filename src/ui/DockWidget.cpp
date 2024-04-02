#include "DockWidget.h"

DockWidget::DockWidget(QWidget* parent) :
    QDockWidget(parent)
{
    delete titleBarWidget();
    setTitleBarWidget(new QWidget);
    setContentsMargins(4, 4, 4, 4);
    setStyleSheet("DockWidget{border:1px solid black;}");
    setFeatures(QDockWidget::NoDockWidgetFeatures);
}

DockWidget::~DockWidget()
{

}
