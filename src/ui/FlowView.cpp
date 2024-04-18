#include "FlowView.h"
#include "FlowNodeFactory.h"
#include "FlowConnection.h"
#include <QtMath>
#include <QKeyEvent>
#include <QApplication>
#include <QMimeData>
#include <QIcon>
#include <QGraphicsSceneMouseEvent>
#include "core/DataManager.h"

//////////////////////////////////////////////////////////////////////////

struct FlowView::Private
{
    FlowConnection* m_connection = nullptr;
    QString m_currentUid;
};

FlowView::FlowView(QWidget* parent) :
    GraphicsView(parent), d(new FlowView::Private)
{
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setAcceptDrops(true);
    setShowGrid(true);
    setMoveSceneEnabled(true);
    setZoomEnabled(true);
    setFactorMax(10);
    setFactorMin(0.25);

    auto scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    scene->installEventFilter(this);
    setScene(scene);

    connect(DM_INST, &DataManager::nodeValueChanged, this, &FlowView::onNodeValueChanged);
}

FlowView::~FlowView()
{

}

void FlowView::load(const NodeInfo& root)
{
    scene()->clear();
    for (const auto& node : root.children)
    {
        if (root.name == "main" && node.function.type != FT_System)
            continue;
        addFlowNode(node);
    }

    for (const auto& conn : root.connections)
    {
        auto vec = conn.split("_", false);
        if (vec.size() >= 5)
        {
            auto connectionType = (IO)vec[0].toInt();
            auto node1 = getNode(vec[1]);
            auto dir1 = (Direction)vec[2].toInt();
            auto dir2 = (Direction)vec[3].toInt();
            auto node2 = getNode(vec[4]);
            if (node1 == nullptr || node2 == nullptr)
                continue;
            auto connection = new FlowConnection();
            scene()->addItem(connection);
            connection->setConnectionType(connectionType);
            connection->setPort1(node1->port(dir1));
            connection->setPort2(node2->port(dir2));
        }
    }

    d->m_currentUid = root.uid;
}

void FlowView::addFlowNode(const NodeInfo& nodeInfo)
{
    FlowNode* node = FlowNodeFactory::createFlowNode(nodeInfo);
    if (node == nullptr) return;

    node->setStyle(FlowNode::LinearGradient);
    node->setBackgroundColor(QColor(0xB4D7F3));
    //node->setShadow(true); // TODO: node尺寸变大之后，开启阴影效果图像会截断
    node->setFlag(QGraphicsItem::ItemIsMovable);
    node->setFlag(QGraphicsItem::ItemIsSelectable);
    node->setData(nodeInfo);

    scene()->addItem(node);
}

void FlowView::autoConnect()
{
    auto nodes = selectedNodes();
    for (int i = 0; i < nodes.size() - 1; i++)
    {
        auto node1 = nodes[i];
        auto node2 = nodes[i + 1];
        if (node1->type() == UINodeCondtion || node1->type() == UINodeLoop
                || node2->type() == UINodeCondtion || node2->type() == UINodeLoop) // 跳过条件和循环，需要逻辑的场景
            continue;
        Direction dir1 = Bottom, dir2 = Top;
        IO type = OUT;
#if 0
        auto rc1 = node1->mapRectToScene(node1->boundingRect());
        auto rc2 = node2->mapRectToScene(node2->boundingRect());
        auto center1 = rc1.center();
        auto center2 = rc2.center();
        double degree;
        if (center2.x() == center1.x()) degree = 90;
        else
        {
            auto tan = (center2.y() - center1.y()) / (center2.x() - center1.x());
            degree = tan * 180 / M_PI;
            if (degree < 0) degree += 180;
        }
        if (degree >= 0 && degree < 45)
        {
            dir1 = Right;
            dir2 = Left;
            if (rc2.left() <= rc1.right())
                dir2 = Top;
        }
        else if (degree >= 45 && degree <= 135)
        {
            dir1 = Bottom;
            dir2 = Top;
        }
        else
        {
            dir1 = Left;
            dir2 = Right;
            if (rc2.right() >= rc1.left())
                dir2 = Top;
        }
#endif
        QString connstr = QString("%1_%2_%3_%4_%5")
                          .arg(type)
                          .arg(itemData(node1).toString())
                          .arg(dir1)
                          .arg(dir2)
                          .arg(itemData(node2).toString());
        auto node = DM_INST->node().findChild(d->m_currentUid);
        if (node && !node->connections.contains(connstr))
        {
            auto connection = new FlowConnection();
            scene()->addItem(connection);
            connection->setPort1(node1->port(dir1));
            connection->setPort2(node2->port(dir2));
            connection->setConnectionType(type);

            node->connections.append(connstr);
            emit DM_INST->connectionAdded(d->m_currentUid, connstr);
        }
    }
}

void FlowView::align(Direction direction)
{
    auto nodes = selectedNodes();
    if (nodes.isEmpty()) return;
    QRectF rc = nodes.first()->mapRectToScene(nodes.first()->boundingRect());
    for (auto node : nodes)
    {
        if (node == nodes.first()) continue;
        auto rect = node->mapRectToScene(node->boundingRect());
        switch (direction)
        {
            case Left: node->setX(rc.left()); break;
            case Top: node->setY(rc.top()); break;
            case Right: node->setX(rc.right() - rect.width()); break;
            case Bottom: node->setY(rc.bottom() - rect.height()); break;
            case HCenter:
                node->setX(rc.center().x() - rect.width() / 2);
                break;
            case VCenter:
                node->setY(rc.center().y() - rect.height() / 2);
                break;
        }
    }
}

void FlowView::onNodeValueChanged(const NodeInfo& nodeInfo)
{
    auto node = getNode(nodeInfo.uid);
    if (node) node->setData(nodeInfo);
}

void FlowView::dragEnterEvent(QDragEnterEvent* event)
{
    if (d->m_currentUid.isEmpty()) return;
    auto node = DM_INST->node().findChild(d->m_currentUid);
    if (node && node->function.name != "main" && event->mimeData()->hasFormat(mimeType()))
        event->acceptProposedAction();
}

void FlowView::dragMoveEvent(QDragMoveEvent* event)
{
    if (event->mimeData()->hasFormat(mimeType()))
        event->acceptProposedAction();
}

void FlowView::dropEvent(QDropEvent* event)
{
    if (event->mimeData()->hasFormat(mimeType()))
    {
        QByteArray mimeData = event->mimeData()->data(mimeType());
        QDataStream dataStream(&mimeData, QIODevice::ReadOnly);
        NodeInfoList datas;
        dataStream >> datas;
        for (auto& node : datas)
        {
            node.uid = DataManager::genUUid();
            node.pos = mapToScene(event->pos());
            addFlowNode(node);

            auto parent = DM_INST->node().findChild(d->m_currentUid);
            if (parent) parent->addChild(node);
            emit DM_INST->nodeAdded(d->m_currentUid, QSharedPointer<NodeInfo>(new NodeInfo(node)));
        }
    }
}

void FlowView::mouseMoveEvent(QMouseEvent* event)
{
    if (d->m_connection != nullptr)
    {
        setDragMode(QGraphicsView::NoDrag);
    }
    __super::mouseMoveEvent(event);
}

void FlowView::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete)
    {
        // 根节点页面（main)不允许删除
        if (d->m_currentUid == DM_INST->node().uid)
            return;

        // 需要确保先删除连线，否则会崩溃（删节点时，同时也会删除节点端口，同时与端口相连的连线也会被删除，
        // 故如果先删除节点的话，会出现两次删除连线，从而导致崩溃）
        auto selections = scene()->selectedItems();
        qSort(selections.begin(), selections.end(), [](QGraphicsItem * lh, QGraphicsItem * rh)
        {
            return lh->type() == UIConnection;
        });
        for (auto item : selections)
        {
            switch (item->type())
            {
                case UINodeFunction:
                case UINodeCondtion:
                case UINodeLoop:
                    {
                        auto uid = itemData(item).toString();
                        DM_INST->node().removeChild(uid);
                    } break;
                case UIConnection:
                    {
                        auto conn = dynamic_cast<FlowConnection*>(item);
                        auto type = conn->connectionType();
                        auto node1 = conn->port1()->parentItem();
                        auto node2 = conn->port2()->parentItem();
                        auto dir1 = conn->port1()->direction();
                        auto dir2 = conn->port2()->direction();
                        auto constr = QString("%1_%2_%3_%4_%5")
                                      .arg(type)
                                      .arg(itemData(node1).toString())
                                      .arg(dir1)
                                      .arg(dir2)
                                      .arg(itemData(node2).toString());
                        DM_INST->node().removeConnection(constr);
                    } break;
            }
            delete item;
        }
        return;
    }
    __super::keyPressEvent(event);
}

bool FlowView::eventFilter(QObject* watched, QEvent* event)
{
    auto e = (QGraphicsSceneMouseEvent*)event;
    switch (e->type())
    {
        case QEvent::GraphicsSceneMouseDoubleClick:
            {
                auto pos = e->scenePos();
                auto item = itemAt(pos);
                auto uid = itemData(item).toString();
                auto node = DM_INST->node().findChild(uid);
                if (node && node->type == NT_Function && node->function.type != FT_API)
                    emit DM_INST->nodeDoubleClicked(uid, true);
            }
            break;
        case QEvent::GraphicsSceneMousePress:
            {
                auto pos = e->scenePos();
                auto item = itemAt(pos);
                if (item == nullptr)
                {
                    emit DM_INST->nodeClicked("");
                }
                else if (itemIsNode(item))
                {
                    auto uid = itemData(item).toString();
                    emit DM_INST->nodeClicked(uid);
                }
                else if (itemIsType(item, FlowItemType::UIPort))
                {
                    auto port = (FlowPort*)item;
                    d->m_connection = new FlowConnection();
                    d->m_connection->setPort1(port);
                    d->m_connection->setPos2(pos);
                    d->m_connection->setConnectionType(port->io());
                    scene()->addItem(d->m_connection);
                    d->m_connection->setSelected(true);
                    return true;
                }
            }
            break;
        case QEvent::GraphicsSceneMouseMove:
            {
                if (d->m_connection)
                {
                    auto pos = e->scenePos();
                    d->m_connection->setPos2(pos);
                    auto item = itemAt(pos);
                    if (itemIsNode(item))
                        item->setSelected(true);
                    return true;
                }
            }
            break;
        case QEvent::GraphicsSceneMouseRelease:
            {
                if (d->m_connection)
                {
                    bool flag = false;
                    auto pos = e->scenePos();
                    auto item = itemAt(pos);
                    if (itemIsType(item, FlowItemType::UIPort))
                    {
                        auto port1 = d->m_connection->port1();
                        auto port2 = dynamic_cast<FlowPort*>(item);
                        if (port1->parentItem() != port2->parentItem() && !port1->isConnectedTo(port2))
                        {
                            flag = true;
                            d->m_connection->setPort2(port2);
                            d->m_connection->setSelected(false);
                            port1 = d->m_connection->port1();
                            port2 = d->m_connection->port2();
                            auto node1 = port1->parentItem();
                            auto node2 = port2->parentItem();
                            QString connection = QString("%1_%2_%3_%4_%5")
                                                 .arg(d->m_connection->connectionType())
                                                 .arg(itemData(node1).toString())
                                                 .arg(port1->direction())
                                                 .arg(port2->direction())
                                                 .arg(itemData(node2).toString());

                            auto node = DM_INST->node().findChild(d->m_currentUid);
                            if (node) node->connections.append(connection);
                            emit DM_INST->connectionAdded(d->m_currentUid, connection);
                        }
                    }
                    if (!flag) delete d->m_connection;
                    d->m_connection = nullptr;
                    scene()->clearSelection();
                }
            }
            break;
    }
    return __super::eventFilter(watched, event);
}

QList<FlowNode*> FlowView::selectedNodes()
{
    QList<FlowNode*> nodes;
    for (auto& item : scene()->selectedItems())
    {
        if (!itemIsNode(item)) continue;
        nodes.push_back((FlowNode*)item);
    }
    qSort(nodes.begin(), nodes.end(), [](const QGraphicsItem * lh, const QGraphicsItem * rh)
    {
        auto pos1 = lh->mapRectToScene(lh->boundingRect()).center();
        auto pos2 = rh->mapRectToScene(rh->boundingRect()).center();
        if (pos1.y() == pos2.y())
            return pos1.x() < pos2.x();
        else
            return pos1.y() < pos2.y();
    });
    return nodes;
}

FlowNode* FlowView::getNode(const QString& uid)
{
    for (auto item : scene()->items())
    {
        auto data = itemData(item).toString();
        if (data == uid) return dynamic_cast<FlowNode*>(item);
    }
    return nullptr;
}

QGraphicsItem* FlowView::itemAt(const QPointF& pos)
{
    return scene()->itemAt(pos, QTransform());
}

bool FlowView::itemIsType(QGraphicsItem* item, int type)
{
    return item && item->type() == type;
}

bool FlowView::itemIsNode(QGraphicsItem* item)
{
    return item && isFlowNodeType(item->type());
}

QVariant FlowView::itemData(QGraphicsItem* item)
{
    QVariant data;
    if (itemIsNode(item))
    {
        auto node = dynamic_cast<FlowNode*>(item);
        if (node) return node->data(Qt::UserRole);
    }
    return data;
}
