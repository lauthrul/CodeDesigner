#include "ParamPropertyEditDialog.h"
#include "VariableSelectorDialog.h"
#include "GlobalVariablesDialog.h"

ParamPropertyEditDialog::ParamPropertyEditDialog(QWidget* parent) :
    QDialog(parent)
{
    ui.setupUi(this);
    connect(ui.btnInsertVariable, &QPushButton::clicked, this, [&]()
    {
        VariableSelectorDialog dlg(m_typeFilter);
        if (dlg.exec() == QDialog::Accepted)
            ui.textEdit->insertPlainText(dlg.current());
    });
    connect(ui.btnGlobalVaribles, &QPushButton::clicked, this, [&]()
    {
        GlobalVariablesDialog dlg;
        dlg.exec();
    });
}

ParamPropertyEditDialog::~ParamPropertyEditDialog()
{

}

void ParamPropertyEditDialog::setTypeFilter(const QString& typeFilter)
{
    m_typeFilter = typeFilter;
}

void ParamPropertyEditDialog::setValue(const QString& value)
{
    ui.textEdit->setText(value);
}

QString ParamPropertyEditDialog::value()
{
    return ui.textEdit->toPlainText();
}
