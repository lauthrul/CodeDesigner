#pragma once

#include <QDialog>
#include "ui_CustomFunctionDialog.h"
#include "core/DataManager.h"

class CustomFunctionDialog : public QDialog
{
    Q_OBJECT

public:
    CustomFunctionDialog(QWidget *parent = nullptr);
    ~CustomFunctionDialog();

public:
    void init(const Function& func);
    void addRow(const Variable& var);
    Function function() const;
    void preview();

protected slots:
    void onAddParam();
    void onDelParam();

private:
    Ui::CustomFunctionDialog ui;
    Function m_func;
};
