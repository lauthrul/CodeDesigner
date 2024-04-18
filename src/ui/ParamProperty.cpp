#include "ParamProperty.h"
#include "ParamPropertyEditDialog.h"
#include "ParamPropertyEdit.h"
#include "core/DataManager.h"

//////////////////////////////////////////////////////////////////////////

bool traverseProperty(QtProperty* node, QtProperty* root, traversePropertyFunc func, void* userData /*= nullptr*/)
{
    if (!func(node, root, userData)) return false;
    for (auto& child : node->subProperties())
    {
        if (!traverseProperty(child, node, func, userData))
            return false;
    }
    return true;
}

QtProperty* findProperty(QtProperty* prop, const QString& name)
{
    if (prop == nullptr) return nullptr;

    if (prop->propertyName() == name)
        return prop;

    for (auto subProp : prop->subProperties())
    {
        if (subProp->propertyName() == name)
            return subProp;

        subProp = findProperty(subProp, name);
        if (subProp) return subProp;
    }
    return nullptr;
}

QtProperty* findProperty(QtAbstractPropertyManager* manager, const QString& name)
{
    if (manager == nullptr) return nullptr;
    for (auto prop : manager->properties())
    {
        prop = findProperty(prop, name);
        if (prop) return prop;
    }
    return nullptr;
}

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
        disconnect(SIGNAL(valueChanged(QtProperty*, const QVariant&)), this);

        // Step
        property = __super::addProperty(QtVariantPropertyManager::enumTypeId(), name);
        d_ptr->m_propertyToType[property] = propertyType;

        QStringList enumNames =
        {
            "STEP_NEXT", "STEP_STOP", "STEP_PAUSE", "STEP_USERBIN", "HSBin",
        };
        for (auto node : DM_INST->node().children)
        {
            if (node.type == NT_Function && node.function.type == FT_Custom)
                enumNames << QString("STEP(%1)").arg(node.function.name);
        }
        property->setAttribute(QLatin1String("enumNames"), enumNames);

        // SBin
        auto subProp = __super::addProperty(QtVariantPropertyManager::enumTypeId(), "SBin");
        enumNames.clear();
        for (const auto& sbin : DM_INST->sBinCodes())
            enumNames << sbin.first;
        subProp->setAttribute(QLatin1String("enumNames"), enumNames);
        subProp->setEnabled(false);
        property->addSubProperty(subProp);

        // HBin
        subProp = __super::addProperty(QtVariantPropertyManager::enumTypeId(), "HBin");
        enumNames.clear();
        for (const auto& sbin : DM_INST->hBinCodes())
            enumNames << sbin.first;
        subProp->setAttribute(QLatin1String("enumNames"), enumNames);
        subProp->setEnabled(false);
        property->addSubProperty(subProp);

        connect(this, &QtVariantPropertyManager::valueChanged, this, [ = ](QtProperty * prop, const QVariant & val)
        {
            if (prop->propertyName() == name)
            {
                auto propSBin = findProperty(prop, "SBin");
                auto propHBin = findProperty(prop, "HBin");
                if (prop->valueText() == "HSBin")
                {
                    if (propSBin) propSBin->setEnabled(true);
                    if (propHBin) propHBin->setEnabled(true);
                }
                else
                {
                    if (propSBin) propSBin->setEnabled(false);
                    if (propHBin) propHBin->setEnabled(false);
                }
            }
        });
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
    traverseProperty(property, nullptr, [ & ](QtProperty * node, QtProperty * root, void* userData)
    {
        d_ptr->m_param[node] = param;
        return true;
    });
}

void ParamPropertyManager::clear() const
{
    d_ptr->m_param.clear();
    d_ptr->m_propertyToType.clear();
    __super::clear();
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
