#include "ToolBox.h"
#include "ui_ToolBox.h"
#include "ToolPage.h"
#include "ToolPageList.h"
#include <QListWidget>
#include <QSpacerItem>

struct ToolBox::Private
{
    QSpacerItem* spacer;
    QList<ToolPage*> pages;
};

ToolBox::ToolBox(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::ToolBox),
    d(new ToolBox::Private)
{
    ui->setupUi(this);
    d->spacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
    connect(ui->lineEditSearch, &QLineEdit::textChanged, this, &ToolBox::onSearch);
}

ToolBox::~ToolBox()
{
    delete d->spacer;
    delete ui;
}

void ToolBox::createListToolPage(const QString& title, const FunctionList& items, bool isExpand /*= false*/)
{
    auto page = new ToolPage(this);
    page->setTitle(title);
    auto widget = new ToolPageList(page);
    widget->setStyleSheet("QListWidget{border:none;}");
    for (auto item : items)
    {
        auto uri = ":/images/icon_fx.png";
        QIcon icon(uri);
        icon.setThemeName(uri);
        widget->addListItem(item, icon);
    }
    page->setWidget(widget);
    addToolPage(page, isExpand);
}

void ToolBox::addToolPage(ToolPage* page, bool isExpand)
{
    connect(page, &ToolPage::toolPageExpandChanged, this, &ToolBox::onToolPageExpandChanged);
    ui->verticalLayoutContent->removeItem(d->spacer);
    ui->verticalLayoutContent->addWidget(page);
    if (isExpand) page->expand();
    else page->collapse();
    d->pages.append(page);
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
        ui->verticalLayoutContent->addItem(d->spacer);
    else
        ui->verticalLayoutContent->removeItem(d->spacer);
}

void ToolBox::onSearch(const QString& text)
{
    for (auto& page : d->pages)
    {
        auto widget = page->widget();
        auto listWidget = dynamic_cast<QListWidget*>(widget);
        if (listWidget)
        {
            auto flag = false;
            for (int i = 0; i < listWidget->count(); i++)
            {
                auto item = listWidget->item(i);
                if (!item->text().contains(text, Qt::CaseInsensitive))
                    item->setHidden(true);
                else
                {
                    item->setHidden(false);
                    flag = true;
                }
            }
            if (flag) page->expand();
        }
    }
}
