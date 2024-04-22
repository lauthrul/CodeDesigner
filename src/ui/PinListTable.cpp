#include "PinListTable.h"
#include <QListWidgetItem>

//////////////////////////////////////////////////////////////////////////

PinListTable::PinListTable(QWidget* parent /*= nullptr*/) : QTableWidget(parent)
{
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setDragEnabled(true);
}

PinListTable::~PinListTable()
{

}

QStringList PinListTable::mimeTypes() const
{
    return { mimeType() };
}

QMimeData* PinListTable::mimeData(const QList<QTableWidgetItem*> items) const
{
    auto mimeData = new QMimeData();
    QByteArray minData;
    QDataStream dataStream(&minData, QIODevice::WriteOnly);
    PinList datas;
    QSet<int> rows;
    for (auto item : items)
    {
        if (item == nullptr)
            continue;
        rows << item->row();
    }
    for (auto row : rows)
        if (m_data) datas.push_back(m_data->value(row));
    dataStream << datas;
    mimeData->setData(mimeTypes()[0], minData);
    return mimeData;
}
