#pragma once

#include <QTableWidget>
#include <QMimeData>
#include "core/Models.h"

//////////////////////////////////////////////////////////////////////////

class PinListTable : public QTableWidget
{
public:
    PinListTable(QWidget* parent = nullptr);
    ~PinListTable();

    static QString mimeType() { return "application/x-pinlist"; };
    void setData(PinList* data) { m_data = data; };

protected:
    virtual QStringList mimeTypes() const override;
    virtual QMimeData* mimeData(const QList<QTableWidgetItem*> items) const override;

private:
    PinList* m_data;
};
