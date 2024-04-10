#include "ParamPropertyEdit.h"
#include "ParamPropertyEditDialog.h"

struct  ParamPropertyEdit::Private
{
    ParamPropertyManager* m_manager;
    QtProperty* m_property;
};

ParamPropertyEdit::ParamPropertyEdit(ParamPropertyManager* manager, QtProperty* property, QWidget* parent) :
    AbstractButtonEdit(parent),
    d(new ParamPropertyEdit::Private)
{
    d->m_manager = manager;
    d->m_property = property;

    ui.lineEdit->setText(value());
    connect(ui.lineEdit, &QLineEdit::textChanged, this, [&](const QString & text)
    {
        setValue(text);
    });
}

ParamPropertyEdit::~ParamPropertyEdit()
{

}

QString ParamPropertyEdit::value() const
{
    return d->m_manager->value(d->m_property).toString();
}

void ParamPropertyEdit::setValue(const QString& value)
{
    d->m_manager->setValue(d->m_property, value);
    emit d->m_manager->propertyChanged(d->m_property);
    emit d->m_manager->valueChanged(d->m_property, value);
}

void ParamPropertyEdit::onButtonClicked()
{
    ParamPropertyEditDialog dlg;
    dlg.setTypeFilter(d->m_manager->param(d->m_property).second.type);
    dlg.setValue(value());
    if (dlg.exec() == QDialog::Accepted)
    {
        auto value = dlg.value();
        ui.lineEdit->setText(value);
        setValue(value);
    }
}
