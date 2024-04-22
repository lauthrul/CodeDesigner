#pragma once

#include <QGroupBox>
#include "ui_DecEditorGroupWidget.h"
#include "core/Models.h"

class DecEditorGroupWidget : public QGroupBox
{
    Q_OBJECT

public:
    DecEditorGroupWidget(QWidget *parent = nullptr);
    ~DecEditorGroupWidget();

public:
    void setData(const PinGroupList& data);
    PinGroupList data() const;

public slots:
    void onAddGroup();

private:
    Ui::DecEditorGroupWidget ui;
};
