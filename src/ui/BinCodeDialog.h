#pragma once

#include <QDialog>
#include "ui_BinCodeDialog.h"
#include "core/DataManager.h"

class BinCodeDialog : public QDialog
{
    Q_OBJECT

public:
    BinCodeDialog(QWidget *parent = nullptr);
    ~BinCodeDialog();

protected slots:
    void onAddRow(QTableWidget* table, const BinCode& data = {});
    void onRmoveRow(QTableWidget* table);
    void onButtonBoxClicked(QAbstractButton* button);

private:
    Ui::BinCodeDialog ui;
};
