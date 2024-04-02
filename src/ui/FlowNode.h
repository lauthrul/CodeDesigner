#pragma once

#include <QObject>
#include <QGraphicsPathItem>
#include <QFont>
#include <QIcon>
#include "FlowPort.h"

class FlowNode : public QObject, public QGraphicsPathItem
{
    Q_OBJECT
    Q_PROPERTY(Style style READ style WRITE setStyle)
    Q_PROPERTY(QFont font READ font WRITE setFont)
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon)
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)
    Q_PROPERTY(bool extend READ extend WRITE setExtend)
    Q_PROPERTY(bool shadow READ shadow WRITE setShadow)

public:
    explicit FlowNode(QGraphicsItem* parent = 0);
    ~FlowNode();

public:
    virtual int type() const override { return FlowItemType::FTNode; }
    virtual QRectF boundingRect() const override;

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
    bool extend() const;
    void setExtend(bool b);
    bool shadow() const;
    void setShadow(bool b);

    void addPort(FlowPort* port);
    FlowPort* port(Direction direction);
    void adjustPortPos(FlowPort* port);
    void showPort(bool show);

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
    struct Private;
    QSharedPointer<Private> d;
};
