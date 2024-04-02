#include "PropertyWidget.h"
#include "core/Models.h"
#include <QHeaderView>
#include "core/DataManager.h"

struct PropertyWidget::Private
{
    QtVariantPropertyManager* m_readonlyManager = nullptr;
    QtVariantPropertyManager* m_editableManager = nullptr;
    QtVariantEditorFactory* m_factory = nullptr;
};

PropertyWidget::PropertyWidget(QWidget* parent) :
    QtTreePropertyBrowser(parent),
    d(new PropertyWidget::Private)
{
    setPropertiesWithoutValueMarked(true);
    setRootIsDecorated(false);
    setResizeMode(QtTreePropertyBrowser::Interactive);

    d->m_readonlyManager = new QtVariantPropertyManager(this);
    d->m_editableManager = new QtVariantPropertyManager(this);

    d->m_factory = new QtVariantEditorFactory(this);
    setFactoryForManager(d->m_editableManager, d->m_factory);
}

PropertyWidget::~PropertyWidget()
{

}

void PropertyWidget::currentFlowNodeChanged(const QString& uid)
{
    clear();

    auto* node = DM_INST->currentRootNode().find(uid);
    if (node == nullptr) return;

    auto fnNewPropertyGroup = [&](QtVariantProperty * parent, QtVariantPropertyManager * manager,
                                  const QString & name)
    {
        QtVariantProperty* group = nullptr;
        if (parent == nullptr)
        {
            group = manager->addProperty(QtVariantPropertyManager::groupTypeId(), name);
            addProperty(group);
        }
        else
        {
            group = manager->addProperty(QVariant::String, name);
            parent->addSubProperty(group);
        }
        return group;
    };
    auto fnNewProperty = [](QtVariantProperty * parent, QtVariantPropertyManager * manager,
                            int type, const QString & name, const QVariant & value)
    {
        auto item = manager->addProperty(type, name);
        item->setValue(value);
        parent->addSubProperty(item);
    };

    // basic
    auto groupItem = fnNewPropertyGroup(nullptr, d->m_readonlyManager, tr("Basic"));
    fnNewProperty(groupItem, d->m_readonlyManager, QVariant::String, tr("FuncRaw"), node->function.raw);
    fnNewProperty(groupItem, d->m_readonlyManager, QVariant::String, tr("FuncRetType"), node->function.retType);
    fnNewProperty(groupItem, d->m_readonlyManager, QVariant::String, tr("FuncName"), node->function.name);

    // params
    groupItem = fnNewPropertyGroup(nullptr, d->m_readonlyManager, tr("Params"));
    for (const auto& param : node->function.params)
    {
        auto subGroup = fnNewPropertyGroup(groupItem, d->m_readonlyManager, param.name);
        fnNewProperty(subGroup, d->m_readonlyManager, QVariant::String, tr("ParamType"), param.type);
        fnNewProperty(subGroup, d->m_readonlyManager, QVariant::String, tr("ParamName"), param.name);
        //fnNewProperty(subGroup, d->m_editableManager, QVariant::QVariant, tr("ParamValue"), param.value);
    }
}
