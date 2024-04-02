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
    void currentFlowNodeChanged(const QString& uid);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

protected:
    FlowNode* getNode(const QString& uid);
    QGraphicsItem* itemAt(const QPointF& pos);
    bool itemIsType(QGraphicsItem* item, int type);
    QVariant itemData(QGraphicsItem* item, int type);

private:
    struct Private;
    QSharedPointer<Private> d;
};
