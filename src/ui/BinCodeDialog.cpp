#include "BinCodeDialog.h"
#include <QListWidgetItem>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QMessageBox>
#include <set>
#include <algorithm>

BinCodeDialog::BinCodeDialog(QWidget* parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    ui.tableWidgetSBin->horizontalHeader()->setStretchLastSection(true);
    ui.tableWidgetHBin->horizontalHeader()->setStretchLastSection(true);

    for (const auto& var : DM_INST->sBinCodes())
        onAddRow(ui.tableWidgetSBin, var);
    for (const auto& var : DM_INST->hBinCodes())
        onAddRow(ui.tableWidgetHBin, var);

    connect(ui.btnAddSBin, &QToolButton::clicked, this, [&]() { onAddRow(ui.tableWidgetSBin); });
    connect(ui.btnDelSBin, &QToolButton::clicked, this, [&]() { onRmoveRow(ui.tableWidgetSBin); });
    connect(ui.btnAddHBin, &QToolButton::clicked, this, [&]() { onAddRow(ui.tableWidgetHBin); });
    connect(ui.btnDelHBin, &QToolButton::clicked, this, [&]() { onRmoveRow(ui.tableWidgetHBin); });
    connect(ui.buttonBox, &QDialogButtonBox::clicked, this, &BinCodeDialog::onButtonBoxClicked);
}

BinCodeDialog::~BinCodeDialog()
{

}

void BinCodeDialog::onAddRow(QTableWidget* table, const BinCode& data)
{
    auto row = table->rowCount();
    auto col = 0;

    table->insertRow(row);

    auto fnCreate = [&](const QString & text)
    {
        auto item = new QTableWidgetItem(text);
        table->setItem(row, col++, item);
        return item;
    };

    fnCreate(data.first);
    QString text;
    if (data.second >= 0) text = QString::number(data.second);
    else text = "~" + QString::number(~data.second);
    fnCreate(text);
}

void BinCodeDialog::onRmoveRow(QTableWidget* table)
{
    QList<int> rows;
    for (const auto& item : table->selectedItems())
    {
        auto row = item->row();
        if (!rows.contains(row))
            rows << row;
    }
    qSort(rows.begin(), rows.end(), qGreater<int>());
    for (auto row : rows)
        table->removeRow(row);
}

void BinCodeDialog::onButtonBoxClicked(QAbstractButton* button)
{
    auto fnData = [&](BinCodeList & list, QTableWidget * table)
    {
        for (int i = 0 ; i < table->rowCount(); i++)
        {
            BinCode bincode;
            auto item = table->item(i, 0);
            bincode.first = item->text();

            item = table->item(i, 1);
            auto text = item->text();
            bool ok = false;
            auto data = text.toInt(&ok);
            if (!ok)
            {
                if (text.startsWith("~"))
                    data = text.mid(1).toInt(&ok);
                if (!ok)
                {
                    QMessageBox::warning(this, tr("Warning"), tr("Invalid value, only can be numbers, such as: 1, ~2 ..."));
                    return false;
                }
                data = ~data;
            }
            bincode.second = data;

            list.push_back(bincode);
        }
        return true;
    };

    if (button == ui.buttonBox->buttons()[0]) // accept
    {
        BinCodeList sBins, hBins;
        if (fnData(sBins, ui.tableWidgetSBin)
                && fnData(hBins, ui.tableWidgetHBin))
        {
            DM_INST->setBinCodes(sBins, hBins);
            accept();
        }
    }
    else
    {
        reject();
    }
}