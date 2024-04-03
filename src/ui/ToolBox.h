#pragma once

#include <QWidget>
#include "core/Models.h"

namespace Ui
{
    class ToolBox;
}

class ToolPage;
class ToolBox : public QWidget
{
    Q_OBJECT

public:
    explicit ToolBox(QWidget* parent = nullptr);
    ~ToolBox();

    void createListToolPage(const QString& title, const FunctionList& items, bool isExpand = false);
    void addToolPage(ToolPage* toolPage, bool isExpand = false);

private slots:
    void onToolPageExpandChanged(ToolPage* page, bool expand);
    void onSearch(const QString& text);

private:
    struct Private;
    QScopedPointer<Private> d;
    Ui::ToolBox* ui;
};
