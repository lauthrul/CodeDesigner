﻿#include "FlowNode.h"
#include <QtMath>
#include <QKeyEvent>
#include <QApplication>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include "core/DataManager.h"

const QRectF cDefRect = QRectF(0, 0, DEFAULT_NODE_WIDTH, DEFAULT_NODE_HEIGHT);

struct FlowNode::Private
{
    // properties
    Style m_style = Style::Flat;
    QFont m_font = QFont("Microsoft YaHei", 10);
    QString m_text;
    QIcon m_icon;
    QColor m_textColor = Qt::black;
    QColor m_backgroundColor = Qt::white;
    QColor m_borderColor = Qt::black;
    bool m_border = true;
    bool m_extend = false;
    bool m_shadow = false;
    // datas
    NodeInfo m_data;
    QMap<Direction, FlowPort*> m_ports;
};

FlowNode::FlowNode(const NodeInfo& data, QGraphicsItem* parent /*= 0*/) :
    QGraphicsPathItem(parent), d(new FlowNode::Private)
{
    d->m_data = data;

    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);

    // give a default path, or the drawn image is incomplete
    QPainterPath path;
    path.addRoundedRect(cDefRect, 5, 5);
    __super::setPath(path);
}

FlowNode::~FlowNode()
{

}

QIcon FlowNode::icon() const
{
    return d->m_icon;
}

void FlowNode::setIcon(const QIcon& icon)
{
    d->m_icon = icon;
    update();
}

QString FlowNode::text() const
{
    return d->m_text;
}

void FlowNode::setText(const QString& text)
{
    if (d->m_text != text)
    {
        d->m_text = text;
        update();
    }
}

FlowNode::Style FlowNode::style() const
{
    return d->m_style;
}

void FlowNode::setStyle(const Style& style)
{
    if (d->m_style != style)
    {
        d->m_style = style;
        update();
    }
}

QFont FlowNode::font() const
{
    return d->m_font;
}

void FlowNode::setFont(const QFont& font)
{
    if (d->m_font != font)
    {
        d->m_font = font;
        update();
    }
}

QColor FlowNode::textColor() const
{
    return d->m_textColor;
}

void FlowNode::setTextColor(const QColor& color)
{
    if (d->m_textColor != color)
    {
        d->m_textColor = color;
        update();
    }
}

QColor FlowNode::backgroundColor() const
{
    return d->m_backgroundColor;
}

void FlowNode::setBackgroundColor(const QColor& color)
{
    if (d->m_backgroundColor != color)
    {
        d->m_backgroundColor = color;
        update();
    }
}

QColor FlowNode::borderColor() const
{
    return d->m_borderColor;
}

void FlowNode::setBorderColor(const QColor& color)
{
    if (d->m_borderColor != color)
    {
        d->m_borderColor = color;
        update();
    }
}

bool FlowNode::border() const
{
    return d->m_border;
}

void FlowNode::setBorder(bool b)
{
    if (d->m_border != b)
    {
        d->m_border = b;
        update();
    }
}

bool FlowNode::extend() const
{
    return d->m_extend;
}

void FlowNode::setExtend(bool b)
{
    if (d->m_extend != b)
    {
        d->m_extend = b;
        update();
    }
}

bool FlowNode::shadow() const
{
    return d->m_shadow;
}

void FlowNode::setShadow(bool b)
{
    if (d->m_shadow != b)
    {
        d->m_shadow = b;
        QGraphicsDropShadowEffect* shadowEffect = nullptr;
        if (b)
        {
            shadowEffect = new QGraphicsDropShadowEffect();
            shadowEffect->setBlurRadius(10.0); // 设置模糊半径
            shadowEffect->setColor(QColor(0, 0, 0, 128)); // 设置阴影颜色（带有透明度的黑色）
            shadowEffect->setOffset(5.0, 5.0); // 设置阴影偏移量
        }
        setGraphicsEffect(shadowEffect);
        update();
    }
}

void FlowNode::addPort(FlowPort* port)
{
    d->m_ports[port->direction()] = port;
}

FlowPort* FlowNode::port(Direction direction)
{
    return d->m_ports.value(direction);
}

QMap<Direction, FlowPort*> FlowNode::ports() const
{
    return d->m_ports;
}

void FlowNode::adjustPortPos(FlowPort* port)
{
    auto rect = boundingRect(); //d->m_rect;
    QPointF pos;
    switch (port->direction())
    {
        case Left:
            pos = { 0, rect.height() / 2 };
            break;
        case Top:
            pos = { rect.width() / 2, 0 };
            break;
        case Right:
            pos = { rect.width(), rect.height() / 2 };
            break;
        case Bottom:
            pos = { rect.width() / 2, rect.height() };
            break;
    }
    port->setPos(pos);
}

void FlowNode::showPort(bool show)
{
    for (auto item : childItems())
    {
        auto port = dynamic_cast<FlowPort*>(item);
        if (port)
        {
            adjustPortPos(port);
            if (show) port->show();
            else port->hide();
        }
    }
}

void FlowNode::setData(const NodeInfo& data)
{
    __super::setData(Qt::UserRole, data.uid);
    update();
}

void FlowNode::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget /*= nullptr*/)
{
    // 计算尺寸
    const auto offset = 4;
    const auto iconsize = 28;
    int iconWidth = 0, textWidth = 0, textHeight = 0, extendWidth = 0;
    QPixmap pixmap = d->m_icon.pixmap(iconsize, iconsize);
    if (!pixmap.isNull())
        iconWidth = pixmap.width() + offset * 2;
    if (!d->m_text.isEmpty())
    {
        auto maxWidth = 0;
        auto lines = d->m_text.split("\n");
        QFontMetrics metrics(d->m_font);
        for (const auto& line : lines)
        {
            maxWidth = qMax<int>(metrics.horizontalAdvance(line), maxWidth);
            textHeight += metrics.height();
        }
        textWidth = maxWidth + offset * 2;
        textHeight += offset * 2;
    }
    if (d->m_extend)
        extendWidth = iconsize + offset * 2;
    auto rect = cDefRect;
    auto width = iconWidth + textWidth + extendWidth;
    rect.setWidth(width);
    rect.setHeight(qMax<qreal>(textHeight, rect.height()));
    setPath(rect);

    // 绘制形状及背景
    if (d->m_style == Style::Flat)
    {
        painter->setBrush(QBrush(d->m_backgroundColor));
    }
    else if (d->m_style == Style::LinearGradient)
    {
        QLinearGradient gradient(0, 0, 0, rect.height());
        gradient.setColorAt(0, d->m_backgroundColor.lighter());
        gradient.setColorAt(1, d->m_backgroundColor);
        painter->setBrush(gradient);
    }

    QPen pen;
    if (isSelected())
        pen = QPen(Qt::red, 2, Qt::DashLine);
    else
    {
        if (d->m_border)
            pen = QPen(d->m_borderColor, 2, Qt::SolidLine);
        else
            pen = Qt::NoPen;
    }
    painter->setPen(pen);
    painter->drawPath(path());

    // 绘制图标
    if (!pixmap.isNull())
        painter->drawPixmap(offset, (rect.height() - pixmap.height()) / 2, pixmap);

    // 绘制文本
    auto rc = rect;
    rc.setLeft(iconWidth);
    rc.setWidth(rect.width() - extendWidth);
    if (!d->m_text.isEmpty())
    {
        painter->setPen(QPen(d->m_textColor));
        painter->setBrush(Qt::BrushStyle::NoBrush);
        painter->setFont(d->m_font);
        QTextOption option(Qt::AlignLeft | Qt::AlignVCenter);
        option.setWrapMode(QTextOption::WordWrap);
        if (pixmap.isNull())
            option.setAlignment(Qt::AlignCenter);
        painter->drawText(rc, d->m_text, option);
    }

    // 绘制扩展图标
    if (d->m_extend)
    {
        QIcon icon(":/images/icon_array_right.png");
        pixmap = icon.pixmap(iconsize, iconsize);
        painter->drawPixmap(width - extendWidth, (rect.height() - pixmap.height()) / 2, pixmap);
    }

    // 显示端点
    showPort(isSelected());
}

QVariant FlowNode::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemSceneHasChanged)
        update();
    else if (change == ItemScenePositionHasChanged)
    {
        auto uid = data(Qt::UserRole).toString();

        auto node = DM_INST->node().findChild(uid);
        if (node) node->pos = value.toPointF();
        emit DM_INST->nodePostionChanged(uid, value.toPointF());
    }
    return value;
}
