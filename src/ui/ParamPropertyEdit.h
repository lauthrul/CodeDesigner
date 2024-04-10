#pragma once

#include "AbstractButtonEdit.h"
#include "ParamProperty.h"

class ParamPropertyEdit : public AbstractButtonEdit
{
    Q_OBJECT

public:
    explicit ParamPropertyEdit(ParamPropertyManager* manager, QtProperty* property, QWidget* parent = nullptr);
    ~ParamPropertyEdit();

private:
    QString value() const;
    void setValue(const QString& value);

private Q_SLOTS:
    virtual void onButtonClicked();

private:
    struct Private;
    QSharedPointer<Private> d;
};
