#include "ToolPageList.h"
#include <QMouseEvent>
#include <QMimeData>
#include <QDrag>

ToolPageList::ToolPageList(QWidget* parent) :
    QListWidget(parent)
{
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragOnly);
}

ToolPageList::~ToolPageList()
{
}

void ToolPageList::addListItem(const Function& info, const QIcon& icon)
{
    QListWidgetItem* item = new  QListWidgetItem(icon, info.name);
    item->setSizeHint(QSize(0, 28));
    item->setToolTip(info.name);
    item->setData(Qt::UserRole, QVariant::fromValue(info));
    addItem(item);
}

QMimeData* ToolPageList::mimeData(const QList<QListWidgetItem*> items) const
{
    auto mimeData = new QMimeData();
    QByteArray minData;
    QDataStream dataStream(&minData, QIODevice::WriteOnly);
    QStringList texts;
    QList<QIcon> icons;
    FunctionList datas;
    for (auto item : items)
    {
        texts << item->text();
        icons << item->icon();
        datas << item->data(Qt::UserRole).value<Function>();
    }
    dataStream << texts << icons << datas;
    mimeData->setData(mimeType(), minData);
    return mimeData;
}
