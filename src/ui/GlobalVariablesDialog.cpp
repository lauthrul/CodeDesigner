#include "GlobalVariablesDialog.h"
#include <QListWidgetItem>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <set>
#include <algorithm>

GlobalVariablesDialog::GlobalVariablesDialog(QWidget* parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    auto hheader = ui.tableWidget->horizontalHeader();
    hheader->setSectionResizeMode(QHeaderView::Interactive);
    hheader->setStretchLastSection(true);
    ui.tableWidget->setColumnWidth(0, 150);
    ui.tableWidget->setColumnWidth(1, 150);
    ui.tableWidget->setColumnWidth(2, 150);
    ui.tableWidget->setColumnWidth(3, 150);

    for (const auto& var : DM_INST->vars())
        addRow(var);

    connect(ui.btnAdd, &QPushButton::clicked, this, &GlobalVariablesDialog::onAddVariable);
    connect(ui.btnDel, &QPushButton::clicked, this, &GlobalVariablesDialog::onDelVariable);
    connect(ui.buttonBox, &QDialogButtonBox::clicked, this, &GlobalVariablesDialog::onButtonBoxClicked);
}

GlobalVariablesDialog::~GlobalVariablesDialog()
{

}

void GlobalVariablesDialog::addRow(const Variable& var)
{
    auto row = ui.tableWidget->rowCount();
    auto col = 0;
    const auto margin = 0;

    ui.tableWidget->insertRow(row);

    // col0: Variable Name
    {
        auto item = new QTableWidgetItem();
        ui.tableWidget->setItem(row, col, item);
        auto widget = new QLineEdit();
        widget->setText(var.name);
        widget->setContentsMargins(margin, margin, margin, margin);
        ui.tableWidget->setCellWidget(row, col, widget);
    }

    // col1: Variable Type
    {
        col++;
        auto item = new QTableWidgetItem();
        ui.tableWidget->setItem(row, col, item);
        auto widget = new QComboBox();
        widget->addItems(
        {
            "char",
            "unsigned char",
            "bool",
            "int",
            "unsigned int",
            "short",
            "long",
            "long long",
            "unsigned long long",
            "double",
            "float",
        });
        widget->setCurrentText(var.type);
        widget->setContentsMargins(margin, margin, margin, margin);
        ui.tableWidget->setCellWidget(row, col, widget);
    }

    // col2: Array Size
    {
        col++;
        auto item = new QTableWidgetItem();
        ui.tableWidget->setItem(row, col, item);
        auto widget = new QSpinBox();
        widget->setMinimum(1);
        widget->setValue(var.arrSize);
        widget->setContentsMargins(margin, margin, margin, margin);
        ui.tableWidget->setCellWidget(row, col, widget);
    }

    // col3: Initial Value
    {
        col++;
        auto item = new QTableWidgetItem();
        ui.tableWidget->setItem(row, col, item);
        auto widget = new QLineEdit();
        widget->setText(var.value);
        widget->setContentsMargins(margin, margin, margin, margin);
        ui.tableWidget->setCellWidget(row, col, widget);
    }
}

Variable GlobalVariablesDialog::current() const
{
    auto row = ui.tableWidget->currentRow();
    return DM_INST->vars().at(row);
}

void GlobalVariablesDialog::onAddVariable()
{
    addRow(Variable());
}

void GlobalVariablesDialog::onDelVariable()
{
    QList<int> rows;
    for (const auto& item : ui.tableWidget->selectedItems())
    {
        auto row = item->row();
        if (!rows.contains(row))
            rows << row;
    }
    qSort(rows.begin(), rows.end(), qGreater<int>());
    for (auto row : rows)
        ui.tableWidget->removeRow(row);
}

void GlobalVariablesDialog::onButtonBoxClicked(QAbstractButton* button)
{
    VariableList vars;
    if (button == ui.buttonBox->buttons()[0]) // accept
    {
        for (int i = 0; i < ui.tableWidget->rowCount(); i++)
        {
            Variable var;
            auto col = 0;
            {
                auto widget = dynamic_cast<QLineEdit*>(ui.tableWidget->cellWidget(i, col++));
                if (widget) var.name = widget->text();
            }
            {
                auto widget = dynamic_cast<QComboBox*>(ui.tableWidget->cellWidget(i, col++));
                if (widget) var.type = widget->currentText();
            }
            {
                auto widget = dynamic_cast<QSpinBox*>(ui.tableWidget->cellWidget(i, col++));
                if (widget) var.arrSize = widget->value();
            }
            {
                auto widget = dynamic_cast<QLineEdit*>(ui.tableWidget->cellWidget(i, col++));
                if (widget) var.value = widget->text();
            }
            vars << var;
        }
        DM_INST->setVars(vars);
        accept();
    }
    else
    {
        reject();
    }
}
