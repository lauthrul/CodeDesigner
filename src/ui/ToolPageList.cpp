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

void ToolPageList::addListItem(const NodeInfo& info)
{
    QListWidgetItem* item = new  QListWidgetItem(QIcon(info.icon), info.name);
    item->setToolTip(info.name);
    item->setData(Qt::UserRole, QVariant::fromValue(info));
    addItem(item);
}

QMimeData* ToolPageList::mimeData(const QList<QListWidgetItem*> items) const
{
    auto mimeData = new QMimeData();
    QByteArray minData;
    QDataStream dataStream(&minData, QIODevice::WriteOnly);
    NodeInfoList datas;
    for (auto item : items)
        datas.push_back(item->data(Qt::UserRole).value<NodeInfo>());
    dataStream << datas;
    mimeData->setData(mimeType(), minData);
    return mimeData;
}
