#pragma once

#include <QTreeWidget>
#include <QScopedPointer>

class PinGroupTree : public QTreeWidget
{
public:
    PinGroupTree(QWidget* parent = nullptr);
    ~PinGroupTree();

public:
    QTreeWidgetItem* addTreeRow(const QString& name, QTreeWidgetItem* parent = nullptr);
    QTreeWidgetItem* topItem(QTreeWidgetItem* item);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    struct Private;
    QScopedPointer<Private> d;
};
