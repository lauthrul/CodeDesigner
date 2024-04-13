#pragma once

#include <QWidget>
#include "qtpropertybrowser/qtpropertymanager.h"
#include "qtpropertybrowser/qtvariantproperty.h"
#include "qtpropertybrowser/qttreepropertybrowser.h"
#include "FlowNode.h"

class TreePropertyBrowser : public QtTreePropertyBrowser
{
    Q_OBJECT

public:
    explicit TreePropertyBrowser(QWidget* parent = nullptr);
    ~TreePropertyBrowser();

public slots:
    void onNodeSelectionChanged(const QString& uid);
    void onNodePostionChanged(const QString& uid, const QPointF& pos);
    void onValueChanged(QtProperty* property, const QVariant& val);

private:
    struct Private;
    QSharedPointer<Private> d;
};
