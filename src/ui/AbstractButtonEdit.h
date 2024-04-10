#pragma once

#include <QWidget>
#include "ui_AbstractButtonEdit.h"

class AbstractButtonEdit: public QWidget
{
    Q_OBJECT

public:
    explicit AbstractButtonEdit(QWidget* parent = nullptr);
    ~AbstractButtonEdit();

private Q_SLOTS:
    virtual void onButtonClicked() = 0;

protected:
    Ui::AbstractButtonEdit ui;
};
