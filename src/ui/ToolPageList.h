#pragma once

#include <QListWidget>
#include "core/Models.h"

class ToolPageList : public QListWidget
{
    Q_OBJECT

public:
    explicit ToolPageList(QWidget* parent = nullptr);
    ~ToolPageList();

public:
    static QString mimeType() { return QStringLiteral("application/x-nodedata"); }
    void addListItem(const Function& info, const QIcon& icon = QIcon());

protected:
    QMimeData* mimeData(const QList<QListWidgetItem*> items) const override;
};
