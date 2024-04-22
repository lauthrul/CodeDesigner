#include "PinGroupTree.h"
#include <QToolButton>
#include <QDragEnterEvent>
#include "PinListTable.h"

//////////////////////////////////////////////////////////////////////////

struct PinGroupTree::Private
{
    QTreeWidgetItem* itemUnderCursor = nullptr;
};

PinGroupTree::PinGroupTree(QWidget* parent /*= nullptr*/) : QTreeWidget(parent), d(new Private)
{
    setAcceptDrops(true);
}

PinGroupTree::~PinGroupTree()
{

}

QTreeWidgetItem* PinGroupTree::addTreeRow(const QString& name, QTreeWidgetItem* parent)
{
    if (parent != nullptr)
    {
        for (auto i = 0; i < parent->childCount(); i++)
        {
            auto item = parent->child(i);
            if (parent->child(i)->text(0) == name)
                return item;
        }
    }

    auto item = new QTreeWidgetItem();
    item->setText(0, name);
    if (parent == nullptr)
    {
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        addTopLevelItem(item);
    }
    else
        parent->addChild(item);
    item->setExpanded(true);

    auto button = new QToolButton(this);
    button->setIcon(QIcon(":/images/icon_del.png"));
    button->setAutoRaise(true);
    setItemWidget(item, 1, button);
    connect(button, &QToolButton::clicked, this, [&]()
    {
        auto pos = QCursor::pos();
        pos = mapFromGlobal(pos);
        auto item = itemAt(pos);
        if (item)
        {
            removeItemWidget(item, 1);
            delete item;
        }
    });
    return item;
}

QTreeWidgetItem* PinGroupTree::topItem(QTreeWidgetItem* item)
{
    if (item == nullptr) return item;

    auto current = item;
    while (current->parent() != nullptr)
        current = item->parent();
    return current;
}

void PinGroupTree::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat(PinListTable::mimeType()))
        event->acceptProposedAction();
}

void PinGroupTree::dragMoveEvent(QDragMoveEvent* event)
{
    if (d->itemUnderCursor)
    {
        d->itemUnderCursor->setBackgroundColor(0, Qt::transparent);
        d->itemUnderCursor->setBackgroundColor(1, Qt::transparent);
    }
    d->itemUnderCursor = nullptr;

    auto item = topItem(itemAt(event->pos()));
    if (item == nullptr) return;

    item->setBackgroundColor(0, QColor(0x0078D7));
    item->setBackgroundColor(1, QColor(0x0078D7));
    d->itemUnderCursor = item;

    if (event->mimeData()->hasFormat(PinListTable::mimeType()))
        event->acceptProposedAction();
}

void PinGroupTree::dropEvent(QDropEvent* event)
{
    if (d->itemUnderCursor)
    {
        d->itemUnderCursor->setBackgroundColor(0, Qt::transparent);
        d->itemUnderCursor->setBackgroundColor(1, Qt::transparent);
    }
    d->itemUnderCursor = nullptr;

    auto item = topItem(itemAt(event->pos()));
    if (item == nullptr) return;

    if (event->mimeData()->hasFormat(PinListTable::mimeType()))
    {
        QByteArray mimeData = event->mimeData()->data(PinListTable::mimeType());
        QDataStream dataStream(&mimeData, QIODevice::ReadOnly);
        PinList datas;
        dataStream >> datas;
        for (auto& pin : datas)
            addTreeRow(pin.name, item);
    }
}
