#pragma once

#include <QDockWidget>

namespace Ui
{
    class DockWidget;
}

class QVBoxLayout;
class DockWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit DockWidget(QWidget *parent = nullptr);
    ~DockWidget();
};
