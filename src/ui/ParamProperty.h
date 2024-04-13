#pragma once

#include "qtpropertybrowser/qtvariantproperty.h"
#include "core/Models.h"

enum
{
    ParamPropertyType = QVariant::UserType + 1,
    StepPropertyType,
};

//////////////////////////////////////////////////////////////////////////

class ParamPropertyManager : public QtVariantPropertyManager
{
    Q_OBJECT

public:
    explicit ParamPropertyManager(QWidget* parent = nullptr);
    ~ParamPropertyManager();

public:
    virtual QtVariantProperty* addProperty(int propertyType, const QString& name = QString());
    int propertyType(const QtProperty* property) const;

    QPair<int, Function::Param> param(const QtProperty* property);
    void setParam(QtProperty* property, const QPair<int, Function::Param>& param);

private:
    struct Private;
    QSharedPointer<Private> d_ptr;
};

//////////////////////////////////////////////////////////////////////////

class ParamEditorFactory : public QtVariantEditorFactory, public QtAbstractEditorFactory<ParamPropertyManager>
{
    Q_OBJECT

public:
    explicit ParamEditorFactory(QWidget* parent = nullptr);
    ~ParamEditorFactory();

protected:
    virtual void connectPropertyManager(ParamPropertyManager* manager);
    virtual QWidget* createEditor(ParamPropertyManager* manager, QtProperty* property, QWidget* parent);
    virtual void disconnectPropertyManager(ParamPropertyManager* manager);

private:
    struct Private;
    QSharedPointer<Private> d;
};

