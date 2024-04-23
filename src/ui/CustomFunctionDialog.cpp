#include "CustomFunctionDialog.h"
#include <QListWidgetItem>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <set>
#include <algorithm>

const auto margin = 0;

void initTypesCombox(QComboBox* combox, const QString& currentText)
{
    combox->addItems(
    {
        "void",
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
    if (!currentText.isEmpty())
        combox->setCurrentText(currentText);
    combox->setContentsMargins(margin, margin, margin, margin);
}

CustomFunctionDialog::CustomFunctionDialog(QWidget* parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    connect(ui.cmbReturnType, &QComboBox::currentTextChanged, this, [&]() { preview(); });
    initTypesCombox(ui.cmbReturnType, "");

    auto hheader = ui.tableWidget->horizontalHeader();
    hheader->setSectionResizeMode(QHeaderView::Interactive);
    hheader->setStretchLastSection(true);
    ui.tableWidget->setColumnWidth(0, 150);

    connect(ui.edtFunctionName, &QLineEdit::textChanged, this, [&]() { preview(); });
    connect(ui.chkPointer, &QCheckBox::stateChanged, this, [&]() { preview(); });
    connect(ui.btnAddParam, &QPushButton::clicked, this, &CustomFunctionDialog::onAddParam);
    connect(ui.btnDelParam, &QPushButton::clicked, this, &CustomFunctionDialog::onDelParam);
}

CustomFunctionDialog::~CustomFunctionDialog()
{

}

void CustomFunctionDialog::init(const Function& func)
{
    QString text = func.retType;
    if (text.contains("*"))
    {
        ui.chkPointer->setChecked(true);
        text.replace("*", "");
    }
    ui.cmbReturnType->setCurrentText(text);
    ui.edtFunctionName->setText(func.name);
    for (const auto& param : func.params)
    {
        Variable var;
        var.type = param.type;
        if (var.type.contains("*"))
        {
            var.type = text.replace("*", "");
            var.isPointer = true;
        }
        if (var.type.contains("["))
        {
            auto start = var.type.indexOf("[") + 1;
            auto end = var.type.indexOf("]");
            var.arrSize = var.type.mid(start, end - start).toInt();
            var.type = var.type.mid(start);
        }
        var.name = param.name;
        var.value = param.value.toString();
        addRow(var);
    }
}

void CustomFunctionDialog::addRow(const Variable& var)
{
    auto row = ui.tableWidget->rowCount();
    auto col = 0;

    ui.tableWidget->insertRow(row);

    // col0: Variable Name
    {
        auto item = new QTableWidgetItem();
        ui.tableWidget->setItem(row, col, item);
        auto widget = new QLineEdit();
        widget->setContentsMargins(margin, margin, margin, margin);
        ui.tableWidget->setCellWidget(row, col, widget);
        connect(widget, &QLineEdit::textChanged, this, [&]() { preview(); });
        widget->setText(var.name);
    }

    // col1: Variable Type
    {
        col++;
        auto item = new QTableWidgetItem();
        ui.tableWidget->setItem(row, col, item);
        auto widget = new QComboBox();
        initTypesCombox(widget, var.type);
        ui.tableWidget->setCellWidget(row, col, widget);
        connect(widget, &QComboBox::currentTextChanged, this, [&]() { preview(); });
        widget->setCurrentText(var.type);
    }

    // col2: Array Size
    {
        col++;
        auto item = new QTableWidgetItem();
        ui.tableWidget->setItem(row, col, item);
        auto widget = new QSpinBox();
        widget->setMinimum(1);
        widget->setMaximum(65536);
        widget->setContentsMargins(margin, margin, margin, margin);
        ui.tableWidget->setCellWidget(row, col, widget);
        connect(widget, &QSpinBox::textChanged, this, [&]() { preview(); });
        widget->setValue(var.arrSize);
    }

    // col3: Pointer
    {
        col++;
        auto item = new QTableWidgetItem();
        ui.tableWidget->setItem(row, col, item);
        auto widget = new QCheckBox();
        widget->setFixedSize(16, 16);
        widget->setObjectName("isPointer");
        auto layout = new QHBoxLayout();
        layout->setAlignment(Qt::AlignCenter);
        layout->addWidget(widget);
        auto widgetWrapper = new QWidget();
        widgetWrapper->setLayout(layout);
        ui.tableWidget->setCellWidget(row, col, widgetWrapper);
        connect(widget, &QCheckBox::stateChanged, this, [&]() { preview(); });
        widget->setChecked(var.isPointer);
    }

    // col4: Initial Value
    {
        col++;
        auto item = new QTableWidgetItem();
        ui.tableWidget->setItem(row, col, item);
        auto widget = new QLineEdit();
        widget->setContentsMargins(margin, margin, margin, margin);
        ui.tableWidget->setCellWidget(row, col, widget);
        connect(widget, &QLineEdit::textChanged, this, [&]() { preview(); });
        widget->setText(var.value);
    }
}

Function CustomFunctionDialog::function() const
{
    return m_func;
}

void CustomFunctionDialog::onAddParam()
{
    addRow(Variable());
    preview();
}

void CustomFunctionDialog::onDelParam()
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
    preview();
}

void CustomFunctionDialog::preview()
{
    m_func.type = FunctionType::FT_Custom;
    m_func.retType = ui.cmbReturnType->currentText() + (ui.chkPointer->isChecked() ? "*" : "");
    m_func.name = ui.edtFunctionName->text();
    m_func.raw = m_func.retType + " " + m_func.name + "(";
    m_func.params.clear();
    QStringList list;
    for (int i = 0; i < ui.tableWidget->rowCount(); i++)
    {
        Function::Param param;
        {
            auto widget = (QLineEdit*)ui.tableWidget->cellWidget(i, 0);
            if (widget) param.name = widget->text();
        }
        {
            auto widget = (QComboBox*)ui.tableWidget->cellWidget(i, 1);
            if (widget) param.type = widget->currentText();
        }
        {
            auto widget = (QSpinBox*)ui.tableWidget->cellWidget(i, 2);
            if (widget && widget->value() > 1)
                param.type += QString("[%1]").arg(widget->value());
        }
        {
            auto widget = (QCheckBox*)ui.tableWidget->cellWidget(i, 3);
            if (widget) widget = widget->findChild<QCheckBox*>("isPointer");
            if (widget && widget->isChecked())
                param.type += "*";
        }
        {
            auto widget = (QLineEdit*)ui.tableWidget->cellWidget(i, 4);
            if (widget && !widget->text().isEmpty())
                param.value = widget->text();
        }
        m_func.params << param;
        auto paramstr = param.type + " " + param.name;
        if (!param.value.toString().isEmpty())
            paramstr += " = " + param.value.toString();
        list << paramstr;
    }
    m_func.raw += list.join(", ") + ")";
    ui.edtPreview->setText(m_func.raw);
}
