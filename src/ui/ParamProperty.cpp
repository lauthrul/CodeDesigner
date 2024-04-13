﻿#include "ParamProperty.h"
#include "ParamPropertyEditDialog.h"
#include "ParamPropertyEdit.h"
#include "core/DataManager.h"

//////////////////////////////////////////////////////////////////////////

struct ParamPropertyManager::Private
{
    QMap<const QtProperty*, int> m_propertyToType;
    QMap<const QtProperty*, QPair<int, Function::Param>> m_param;
};

ParamPropertyManager::ParamPropertyManager(QWidget* parent) :
    QtVariantPropertyManager(parent),
    d_ptr(new ParamPropertyManager::Private)
{
}

ParamPropertyManager::~ParamPropertyManager()
{

}

QtVariantProperty* ParamPropertyManager::addProperty(int propertyType, const QString& name /*= QString()*/)
{
    QtVariantProperty* property = nullptr;
    if (propertyType == ParamPropertyType)
    {
        property = __super::addProperty(QVariant::String, name);
        d_ptr->m_propertyToType[property] = propertyType;
    }
    else if (propertyType == StepPropertyType)
    {
        property = __super::addProperty(QtVariantPropertyManager::enumTypeId(), name);
        d_ptr->m_propertyToType[property] = propertyType;
        QStringList enumNames =
        {
            "STEP_NEXT", "STEP_STOP", "STEP_PAUSE", "STEP_USERBIN",
        };
        traverseNodeInfo(&DM_INST->node(), nullptr, [&](NodeInfo * node, NodeInfo * parent, void* userData)
        {
            if (node->type == NT_Function && node->function.type == FT_Custom)
                enumNames << QString("STEP(%1)").arg(node->function.name);
            return true;
        }, nullptr);
        property->setAttribute(QLatin1String("enumNames"), enumNames);
    }
    else property = __super::addProperty(propertyType, name);
    return property;
}

int ParamPropertyManager::propertyType(const QtProperty* property) const
{
    auto it = d_ptr->m_propertyToType.constFind(property);
    if (it != d_ptr->m_propertyToType.constEnd())
        return it.value();
    return __super::propertyType(property);
}

QPair<int, Function::Param> ParamPropertyManager::param(const QtProperty* property)
{
    if (d_ptr->m_param.contains(property))
        return d_ptr->m_param[property];
    return { -1, Function::Param() };
}

void ParamPropertyManager::setParam(QtProperty* property, const QPair<int, Function::Param>& param)
{
    d_ptr->m_param[property] = param;
}

//////////////////////////////////////////////////////////////////////////

struct ParamEditorFactory::Private
{
};

ParamEditorFactory::ParamEditorFactory(QWidget* parent) :
    QtVariantEditorFactory(parent),
    QtAbstractEditorFactory<ParamPropertyManager>(parent),
    d(new ParamEditorFactory::Private)
{
}

ParamEditorFactory::~ParamEditorFactory()
{

}

void ParamEditorFactory::connectPropertyManager(ParamPropertyManager* manager)
{
    QtVariantEditorFactory::connectPropertyManager(manager);
}

QWidget* ParamEditorFactory::createEditor(ParamPropertyManager* manager, QtProperty* property, QWidget* parent)
{
    if (manager->propertyType(property) == ParamPropertyType)
        return new ParamPropertyEdit(manager, property, parent);
    return QtVariantEditorFactory::createEditor(manager, property, parent);
}

void ParamEditorFactory::disconnectPropertyManager(ParamPropertyManager* manager)
{
    QtVariantEditorFactory::disconnectPropertyManager(manager);
}
