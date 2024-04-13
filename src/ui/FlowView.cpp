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
#include "core/util.h"

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
        std::vector<std::string> vec;
        if (util::str::split(vec, conn.toStdString().c_str(), "_", false) >= 4)
        {
            auto node1 = getNode(vec[0].c_str());
            auto direction1 = (Direction)atoi(vec[1].c_str());
            auto direction2 = (Direction)atoi(vec[2].c_str());
            auto node2 = getNode(vec[3].c_str());
            auto connection = new FlowConnection();
            scene()->addItem(connection);
            connection->setPort1(node1->port(direction1));
            connection->setPort2(node2->port(direction2));
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

    node->addPort(new FlowPort(Left, node));
    node->addPort(new FlowPort(Top, node));
    node->addPort(new FlowPort(Right, node));
    node->addPort(new FlowPort(Bottom, node));

    scene()->addItem(node);
}

void FlowView::align(Direction direction)
{
    QList<QGraphicsItem*> nodes;
    QRectF rc;
    for (auto& item : scene()->selectedItems())
    {
        if (!itemIsNode(item)) continue;
        if (rc.isEmpty())
            rc = item->mapRectToScene(item->boundingRect());
        nodes.push_back(item);
    }

    for (auto node : nodes)
    {
        auto rect = node->boundingRect();
        switch (direction)
        {
            case Left: node->setX(rc.left()); break;
            case Top: node->setY(rc.top()); break;
            case Right: node->setX(rc.right() - rect.width()); break;
            case Bottom: node->setY(rc.bottom() - rect.height()); break;
            case HCenter:
                {
                    auto center = (rc.right() - rc.left()) / 2;
                    node->setX(center - rect.width() / 2);
                }
                break;
            case VCenter:
                {
                    auto center = (rc.bottom() - rc.top()) / 2;
                    node->setY(center - rect.height() / 2);
                }
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
        for (auto item : scene()->selectedItems())
        {
            switch (item->type())
            {
                case NodeFunctionType:
                case NodeCondtionType:
                case NodeLoopType:
                    {
                        auto uid = itemData(item).toString();
                        auto node = DM_INST->node().findChild(uid);
                        if (node && node->type == NT_Function && node->function.type == FT_System)
                            return; // 系统函数不允许删除
                        DM_INST->node().removeChild(uid);
                    } break;
                case ConnectionType:
                    {
                        auto conn = dynamic_cast<FlowConnection*>(item);
                        auto node1 = conn->port1()->parentItem();
                        auto node2 = conn->port2()->parentItem();
                        auto dir1 = conn->port1()->direction();
                        auto dir2 = conn->port2()->direction();
                        auto constr = QString("%1_%2_%3_%4")
                                      .arg(itemData(node1).toString())
                                      .arg(dir1)
                                      .arg(dir2)
                                      .arg(itemData(node2).toString());
                        DM_INST->node().removeConnection(constr);
                    } break;
            }
        }
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
                else if (itemIsType(item, FlowItemType::PortType))
                {
                    d->m_connection = new FlowConnection();
                    scene()->addItem(d->m_connection);
                    auto uid = itemData(item->parentItem()).toString();
                    auto node = DM_INST->node().findChild(uid);
                    if (node && (node->type == NT_Condtion || node->type == NT_Loop))
                        d->m_connection->setText(tr("TRUE"));
                    d->m_connection->setPort1((FlowPort*)item);
                    d->m_connection->setPos2(pos);
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
                    if (itemIsType(item, FlowItemType::PortType))
                    {
                        auto port1 = d->m_connection->port1();
                        auto port2 = dynamic_cast<FlowPort*>(item);
                        if (port1->parentItem() != port2->parentItem() && !port1->isConnectedTo(port2))
                        {
                            d->m_connection->setPort2(port2);
                            d->m_connection->setSelected(false);
                            flag = true;
                            auto node1 = port1->parentItem();
                            auto node2 = port2->parentItem();
                            QString connection = QString("%1_%2_%3_%4")
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
