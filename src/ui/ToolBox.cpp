#include "ToolBox.h"
#include "ui_ToolBox.h"
#include "ToolPage.h"
#include "ToolPageList.h"
#include <QListWidget>
#include <QSplitter>
#include <QSpacerItem>

struct ToolBox::Private
{
    QSplitter* splitter;
    QSpacerItem* spacer;
    QList<ToolPage*> pages;
};

ToolBox::ToolBox(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::ToolBox),
    d(new ToolBox::Private)
{
    ui->setupUi(this);

    d->splitter = new QSplitter(Qt::Vertical);
    d->splitter->setChildrenCollapsible(false);
    d->splitter->show();
    ui->verticalLayoutContent->addWidget(d->splitter);

    d->spacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);

    connect(ui->lineEditSearch, &QLineEdit::textChanged, this, &ToolBox::onSearch);
}

ToolBox::~ToolBox()
{
    delete d->splitter;
    delete d->spacer;
    delete ui;
}

ToolPage* ToolBox::createListToolPage(const QString& title, const NodeInfoList& items, int viewMode, bool isExpand /*= false*/)
{
    auto page = new ToolPage(this);
    page->setTitle(title);
    auto widget = new ToolPageList(page);
    widget->setResizeMode(QListView::Adjust);
    widget->setViewMode(((QListView::ViewMode)viewMode));
    widget->setIconSize(QSize(28, 28));
    widget->setStyleSheet("QListWidget{border:none;}");
    for (auto item : items)
        widget->addListItem(item);
    page->setWidget(widget);
    addToolPage(page, isExpand);
    return page;
}

void ToolBox::addToolPage(ToolPage* page, bool isExpand)
{
    connect(page, &ToolPage::toolPageExpandChanged, this, &ToolBox::onToolPageExpandChanged);
    ui->verticalLayoutContent->removeItem(d->spacer);
    d->splitter->addWidget(page);
    if (isExpand) page->expand();
    else page->collapse();
    d->pages.append(page);
}

void ToolBox::removeToolPage(ToolPage* toolPage)
{
    if (toolPage)
    {
        d->pages.removeAll(toolPage);
        toolPage->setParent(nullptr);
        delete toolPage;
    }
}

void ToolBox::onToolPageExpandChanged(ToolPage* page, bool expand)
{
    auto allCollapse = true;
    for (int i = 0; i < d->splitter->count(); i++)
    {
        auto widget = d->splitter->widget(i);
        auto page = dynamic_cast<ToolPage*>(widget);
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
        if (d->pages.indexOf(page) == 0)
            continue;
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
