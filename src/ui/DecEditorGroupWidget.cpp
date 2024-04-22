#include "DecEditorGroupWidget.h"
#include <QInputDialog>

DecEditorGroupWidget::DecEditorGroupWidget(QWidget* parent)
    : QGroupBox(parent)
{
    ui.setupUi(this);

    ui.treePinGroup->setColumnCount(2);
    ui.treePinGroup->setHeaderHidden(true);

    connect(ui.btnAddGroup, &QPushButton::clicked, this, &DecEditorGroupWidget::onAddGroup);
}

DecEditorGroupWidget::~DecEditorGroupWidget()
{

}

void DecEditorGroupWidget::setData(const PinGroupList& data)
{
    for (const auto& d : data)
    {
        auto item = ui.treePinGroup->addTreeRow(d.first);
        for (const auto& v : d.second)
            ui.treePinGroup->addTreeRow(v, item);
    }
}

PinGroupList DecEditorGroupWidget::data() const
{
    PinGroupList list;
    for (int i = 0; i < ui.treePinGroup->topLevelItemCount(); i++)
    {
        auto item = ui.treePinGroup->topLevelItem(i);
        PinGroup group;
        group.first = item->text(0);
        for (int j = 0; j < item->childCount(); j++)
        {
            auto subItem = item->child(j);
            group.second << subItem->text(0);
        }
        list << group;
    }
    return list;
}

void DecEditorGroupWidget::onAddGroup()
{
    auto name = QInputDialog::getText(this, tr("Add Group"), tr("Group Name"));
    if (name.isEmpty()) return;
    ui.treePinGroup->addTreeRow(name);
}
