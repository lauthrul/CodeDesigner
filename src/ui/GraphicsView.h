#pragma once

#include <QGraphicsView>

class GraphicsView : public QGraphicsView
{
    Q_OBJECT
    Q_PROPERTY(double factorMax READ factorMax WRITE setFactorMax)
    Q_PROPERTY(double factorMin READ factorMin WRITE setFactorMin)
    Q_PROPERTY(QColor gridColor READ gridColor WRITE setGridColor)
    Q_PROPERTY(bool showGrid READ showGrid WRITE setShowGrid)
    Q_PROPERTY(bool moveSceneEnabled READ moveSceneEnabled WRITE setMoveSceneEnabled)
    Q_PROPERTY(bool zoomEnabled READ zoomEnabled WRITE setZoomEnabled)

public:
    explicit GraphicsView(QWidget* parent = nullptr);
    ~GraphicsView();

public:
    double factorMax() const;
    void setFactorMax(double val);
    double factorMin() const;
    void setFactorMin(double val);
    QColor gridColor() const;
    void setGridColor(const QColor& color);
    bool showGrid() const;
    void setShowGrid(bool b);
    bool moveSceneEnabled() const;
    void setMoveSceneEnabled(bool b);
    bool zoomEnabled() const;
    void setZoomEnabled(bool b);

    void zoomIn();
    void zoomOut();
    void resetZoom();

protected:
    virtual void drawBackground(QPainter* painter, const QRectF& rect) override;
    virtual void wheelEvent(QWheelEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void keyPressEvent(QKeyEvent* event) override;

private:
    void drawGrid(QPainter* painter, double gridStep);

private:
    struct Private;
    QSharedPointer<Private> d;
};
