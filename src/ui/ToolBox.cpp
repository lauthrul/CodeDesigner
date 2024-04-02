#include "ToolBox.h"
#include "ui_ToolBox.h"
#include "ToolPage.h"
#include "ToolPageList.h"
#include <QListWidget>

ToolBox::ToolBox(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::ToolBox)
{
    ui->setupUi(this);
    spacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
}

ToolBox::~ToolBox()
{
    delete spacer;
    delete ui;
}

void ToolBox::createListToolPage(const QString& title, const FunctionList& items, bool isExpand /*= false*/)
{
    auto page = new ToolPage(this);
    page->setTitle(title);
    auto widget = new ToolPageList(page);
    widget->setStyleSheet("QListWidget{border:none;}");
    for (auto item : items)
        widget->addListItem(item, QIcon(":/images/icon_fx.png"));
    page->setWidget(widget);
    addToolPage(page, isExpand);
}

void ToolBox::addToolPage(ToolPage* page, bool isExpand)
{
    connect(page, &ToolPage::toolPageExpandChanged, this, &ToolBox::onToolPageExpandChanged);
    ui->verticalLayoutContent->removeItem(spacer);
    ui->verticalLayoutContent->addWidget(page);
    if (isExpand) page->expand();
    else page->collapse();
}

void ToolBox::onToolPageExpandChanged(ToolPage* page, bool expand)
{
    auto allCollapse = true;
    for (int i = 0; i < ui->verticalLayoutContent->count(); i++)
    {
        auto item = ui->verticalLayoutContent->itemAt(i);
        auto page = dynamic_cast<ToolPage*>(item->widget());
        if (page && page->isExpand())
        {
            allCollapse = false;
            break;
        }
    }
    if (allCollapse)
        ui->verticalLayoutContent->addItem(spacer);
    else
        ui->verticalLayoutContent->removeItem(spacer);
}
