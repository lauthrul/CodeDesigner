#pragma once

#include <QDialog>
#include "ui_VariableSelectorDialog.h"
#include "core/DataManager.h"

class VariableSelectorDialog : public QDialog
{
    Q_OBJECT

public:
    VariableSelectorDialog(const QString& type, QWidget *parent = nullptr);
    ~VariableSelectorDialog();

public:
    void addRow(const Variable& var);
    QString current() const;

private:
    Ui::VariableSelectorDialog ui;
};
