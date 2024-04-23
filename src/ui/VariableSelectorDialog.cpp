#include "VariableSelectorDialog.h"
#include <QListWidgetItem>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <set>
#include <algorithm>

VariableSelectorDialog::VariableSelectorDialog(const QString& type, QWidget* parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    auto hheader = ui.tableWidget->horizontalHeader();
    hheader->setSectionResizeMode(QHeaderView::Interactive);
    hheader->setStretchLastSection(true);
    ui.tableWidget->setColumnWidth(0, 120);
    ui.tableWidget->setColumnWidth(1, 80);
    ui.tableWidget->setColumnWidth(2, 80);
    ui.tableWidget->setColumnWidth(3, 80);
    ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    auto title = tr("Variable Selector");
    if (!type.isEmpty())
        title += QString(" (%1)").arg(type);
    setWindowTitle(title);

    for (const auto& var : DM_INST->vars())
    {
        if (type.isEmpty()) addRow(var);
        else
        {
            if (type.startsWith(var.type))
            {
                if (type.count("*") == 0 && var.arrSize == 1)
                    addRow(var);
                else if (type.count("*") == 1 && var.arrSize > 1)
                    addRow(var);
            }
        }
    }

    connect(ui.tableWidget, &QTableWidget::doubleClicked, this, [&](const QModelIndex & index)
    {
        accept();
    });
}

VariableSelectorDialog::~VariableSelectorDialog()
{

}

void VariableSelectorDialog::addRow(const Variable& var)
{
    auto row = ui.tableWidget->rowCount();
    auto col = 0;
    const auto margin = 0;

    ui.tableWidget->insertRow(row);

    auto fnCreate = [&](const QString & text)
    {
        auto item = new QTableWidgetItem(text);
        item->setFlags(item->flags() & (~Qt::ItemIsEditable));
        ui.tableWidget->setItem(row, col++, item);
    };

    fnCreate(var.name);
    fnCreate(var.type);
    fnCreate(QString::number(var.arrSize));
    fnCreate(var.isPointer ? tr("True") : tr("False"));
    fnCreate(var.value);
}

QString VariableSelectorDialog::current() const
{
    auto row = ui.tableWidget->currentRow();
    return ui.tableWidget->item(row, 0)->text();
}
