#pragma once

#include "GraphicsView.h"
#include "FlowNode.h"
#include "core/Models.h"

class FlowView : public GraphicsView
{
    Q_OBJECT

public:
    explicit FlowView(QWidget* parent = nullptr);
    ~FlowView();

public:
    static QString mimeType() { return QStringLiteral("application/x-nodedata"); }

public:
    void load(const NodeInfo& root);
    void addFlowNode(const NodeInfo& node);

signals:
    void nodeSelectionChanged(const QString& uid);

public slots:
    void align(Direction direction);
    void onNodeValueChanged(const NodeInfo& nodeInfo);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

protected:
    FlowNode* getNode(const QString& uid);
    QGraphicsItem* itemAt(const QPointF& pos);
    bool itemIsType(QGraphicsItem* item, int type);
    bool itemIsNode(QGraphicsItem* item);
    QVariant itemData(QGraphicsItem* item);

private:
    struct Private;
    QSharedPointer<Private> d;
};
