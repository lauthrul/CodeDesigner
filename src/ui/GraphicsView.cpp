#include "GraphicsView.h"
#include <QtMath>
#include <QKeyEvent>
#include <QApplication>
#include <QGraphicsPathItem>

struct GraphicsView::Private
{
    // properties
    double m_factorMax;
    double m_factorMin;
    QColor m_gridColor = QColor(240, 240, 240);
    bool m_showGrid = false;
    bool m_moveSceneEnabled = false;
    bool m_zoomEnabled = false;

    // datas
    QPointF m_scenePos;
    QPointF m_pressPos;
    bool m_sceneMoving = false;
    QTransform m_defTransform;
};

GraphicsView::GraphicsView(QWidget* parent) :
    QGraphicsView(parent), d(new GraphicsView::Private)
{
    setRenderHint(QPainter::Antialiasing);
    setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setCacheMode(QGraphicsView::CacheBackground);

    d->m_defTransform = transform();
}

GraphicsView::~GraphicsView()
{
}

double GraphicsView::factorMax() const
{
    return d->m_factorMax;
}

void GraphicsView::setFactorMax(double val)
{
    d->m_factorMax = val;
}

double GraphicsView::factorMin() const
{
    return d->m_factorMin;
}

void GraphicsView::setFactorMin(double val)
{
    d->m_factorMin = val;
}

QColor GraphicsView::gridColor() const
{
    return d->m_gridColor;
}

void GraphicsView::setGridColor(const QColor& color)
{
    if (d->m_gridColor != color)
    {
        d->m_gridColor = color;
        update();
    }
}

bool GraphicsView::showGrid() const
{
    return d->m_showGrid;
}

void GraphicsView::setShowGrid(bool b)
{
    if (d->m_showGrid != b)
    {
        d->m_showGrid = b;
        update();
    }
}

bool GraphicsView::moveSceneEnabled() const
{
    return d->m_moveSceneEnabled;
}

void GraphicsView::setMoveSceneEnabled(bool b)
{
    d->m_moveSceneEnabled = b;
}

bool GraphicsView::zoomEnabled() const
{
    return d->m_zoomEnabled;
}

void GraphicsView::setZoomEnabled(bool b)
{
    d->m_zoomEnabled = b;
}

void GraphicsView::zoomIn()
{
    qreal factor = transform().scale(1.2, 1.2).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor > d->m_factorMax) return; /* 防止视图过大 */
    scale(1.2, 1.2);
}

void GraphicsView::zoomOut()
{
    qreal factor = transform().scale(1 / 1.2, 1 / 1.2).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < d->m_factorMin) return; /* 防止视图过小 */
    scale(1 / 1.2, 1 / 1.2);
}

void GraphicsView::resetZoom()
{
    setTransform(d->m_defTransform);
}

void GraphicsView::drawBackground(QPainter* painter, const QRectF& rect)
{
    QGraphicsView::drawBackground(painter, rect);
    if (d->m_showGrid)
    {
        QPen pen(d->m_gridColor, 0.6);
        painter->setPen(pen);
        drawGrid(painter, 15);

        pen.setWidth(2.0);
        painter->setPen(pen);
        drawGrid(painter, 150);
    }
}

void GraphicsView::drawGrid(QPainter* painter, double gridStep)
{
    QRect   windowRect = rect();
    QPointF tl = mapToScene(windowRect.topLeft());
    QPointF br = mapToScene(windowRect.bottomRight());

    double left = qFloor(tl.x() / gridStep - 0.5);
    double right = qFloor(br.x() / gridStep + 1.0);
    double bottom = qFloor(tl.y() / gridStep - 0.5);
    double top = qFloor(br.y() / gridStep + 1.0);

    for (int xi = int(left); xi <= int(right); ++xi)
    {
        QLineF line(xi * gridStep, bottom * gridStep,
                    xi * gridStep, top * gridStep);

        painter->drawLine(line);
    }

    for (int yi = int(bottom); yi <= int(top); ++yi)
    {
        QLineF line(left * gridStep, yi * gridStep,
                    right * gridStep, yi * gridStep);
        painter->drawLine(line);
    }
}

void GraphicsView::wheelEvent(QWheelEvent* event)
{
    if (d->m_zoomEnabled)
    {
        if (event->delta() > 0) zoomIn();
        else zoomOut();
        update();
    }
    QGraphicsView::wheelEvent(event);
}

void GraphicsView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        setDragMode(QGraphicsView::RubberBandDrag);
    }
    else if (event->button() == Qt::MiddleButton)
    {
        if (d->m_moveSceneEnabled)
        {
            QMouseEvent fake(event->type(), event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());
            d->m_scenePos = mapToScene(event->pos());
            d->m_pressPos = d->m_scenePos;

            setDragMode(QGraphicsView::ScrollHandDrag);
            setInteractive(false);

            event = &fake;

            d->m_sceneMoving = true;
            update();
        }
    }
    QGraphicsView::mousePressEvent(event);
}

void GraphicsView::mouseMoveEvent(QMouseEvent* event)
{
    if (d->m_moveSceneEnabled)
    {
        d->m_scenePos = mapToScene(event->pos());
        if (d->m_sceneMoving)
        {
            QPointF difference = d->m_pressPos - d->m_scenePos;
            setSceneRect(sceneRect().translated(difference.x(), difference.y()));
            update();
        }
    }
    QGraphicsView::mouseMoveEvent(event);
}

void GraphicsView::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MiddleButton)
    {
        if (d->m_moveSceneEnabled)
        {
            QMouseEvent fake(event->type(), event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());

            setDragMode(QGraphicsView::NoDrag);
            setInteractive(true);

            event = &fake;
            d->m_sceneMoving = false;
            update();
        }
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void GraphicsView::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete)
    {
        for (auto item : scene()->selectedItems())
            delete item;
    }
    else if ((event->modifiers() == Qt::ControlModifier) && (event->key() == Qt::Key_A))
    {
        for (auto item : scene()->items())
            item->setSelected(true);
    }
    else if ((event->modifiers() == Qt::ControlModifier) && (event->key() == Qt::Key_0))
    {
        resetZoom();
    }
    else if ((event->modifiers() == Qt::ControlModifier) && (event->key() == Qt::Key_Equal))
    {
        zoomIn();
    }
    else if ((event->modifiers() == Qt::ControlModifier) && (event->key() == Qt::Key_Minus))
    {
        zoomOut();
    }
}
