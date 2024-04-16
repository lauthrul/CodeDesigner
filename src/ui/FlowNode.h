#pragma once

#include <QObject>
#include <QGraphicsPathItem>
#include <QFont>
#include <QIcon>
#include "FlowPort.h"
#include "core/Models.h"

class FlowNode : public QObject, public QGraphicsPathItem
{
    Q_OBJECT
public:
    explicit FlowNode(const NodeInfo& data, QGraphicsItem* parent = 0);
    ~FlowNode();

public:
    QIcon icon() const;
    void setIcon(const QIcon& icon);
    QString text() const;
    void setText(const QString& text);
    enum Style { Flat, LinearGradient, };
    Q_ENUM(Style)
    Style style() const;
    void setStyle(const Style& style);
    QFont font() const;
    void setFont(const QFont& font);
    QColor textColor() const;
    void setTextColor(const QColor& color);
    QColor backgroundColor() const;
    void setBackgroundColor(const QColor& color);

    QColor borderColor() const;
    void setBorderColor(const QColor& color);
    bool border() const;
    void setBorder(bool b);

    bool extend() const;
    void setExtend(bool b);
    bool shadow() const;
    void setShadow(bool b);

    void addPort(FlowPort* port);
    FlowPort* port(Direction direction);
    QMap<Direction, FlowPort*> ports() const;
    void adjustPortPos(FlowPort* port);
    void showPort(bool show);

public:
    virtual void setData(const NodeInfo& data);
    virtual void setPath(QRectF& rc) = 0;

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
    struct Private;
    QSharedPointer<Private> d;
};
