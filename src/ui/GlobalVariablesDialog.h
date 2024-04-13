#pragma once

#include <QDialog>
#include "ui_GlobalVariablesDialog.h"
#include "core/DataManager.h"

class GlobalVariablesDialog : public QDialog
{
    Q_OBJECT

public:
    GlobalVariablesDialog(QWidget *parent = nullptr);
    ~GlobalVariablesDialog();

public:
    void addRow(const Variable& var);
    Variable current() const;

protected slots:
    void onAddVariable();
    void onDelVariable();
    void onButtonBoxClicked(QAbstractButton* button);

private:
    Ui::GlobalVariablesDialog ui;
};
