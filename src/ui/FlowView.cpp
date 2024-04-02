#include "FlowView.h"
#include "FlowNode.h"
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
}

FlowView::~FlowView()
{

}

void FlowView::load(const NodeInfo& root)
{
    scene()->clear();
    for (const auto& node : root.children)
        addFlowNode(node);

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
    FlowNode* node = new FlowNode();
    node->setIcon(nodeInfo.icon);
    node->setText(nodeInfo.function.name);
    node->setTextColor(Qt::white);
    node->setStyle(FlowNode::LinearGradient);
    node->setBackgroundColor(QColor(0, 122, 204));
    node->setShadow(true);
    node->setFlag(QGraphicsItem::ItemIsMovable);
    node->setFlag(QGraphicsItem::ItemIsSelectable);
    node->setPos(nodeInfo.pos);
    node->setData(Qt::UserRole, nodeInfo.uid);
    node->setExtend(nodeInfo.function.type != Function::API);

    node->addPort(new FlowPort(Left, node));
    node->addPort(new FlowPort(Top, node));
    node->addPort(new FlowPort(Right, node));
    node->addPort(new FlowPort(Bottom, node));

    scene()->addItem(node);
}

void FlowView::dragEnterEvent(QDragEnterEvent* event)
{
    auto node = DM_INST->currentRootNode().find(d->m_currentUid);
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
        QStringList texts;
        QList<QIcon> icons;
        FunctionList datas;
        dataStream >> texts >> icons >> datas;
        for (int i = 0; i < texts.size(); i++)
        {
            const auto& text = texts[i];
            const auto& icon = icons[i];
            const auto& data = datas[i];

            NodeInfo node;
            node.uid = DataManager::genUUid();
            node.icon = icon;
            node.function = data;
            node.pos = mapToScene(event->pos());

            addFlowNode(node);

            emit DM_INST->nodeAdded(QSharedPointer<NodeInfo>(new NodeInfo(node)));
        }
    }
}

void FlowView::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete)
    {
        for (auto item : scene()->selectedItems())
        {
            auto uid = itemData(item, FlowItemType::FTNode).toString();
            auto node = DM_INST->currentRootNode().find(uid);
            if (node && node->function.type == Function::System)
                return; // 系统函数不允许删除

            DM_INST->currentRootNode().removeByUid(uid);
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
                auto data = itemData(item, FlowItemType::FTNode);
                auto uid = data.toString();
                auto node = DM_INST->currentRootNode().find(uid);
                if (node && node->function.type != Function::API)
                    emit DM_INST->nodeSwitched(uid, true);
            }
            break;
        case QEvent::GraphicsSceneMousePress:
            {
                auto pos = e->scenePos();
                auto item = itemAt(pos);
                if (itemIsType(item, FlowItemType::FTNode))
                {
                    auto uid = itemData(item, FlowItemType::FTNode).toString();
                    emit currentFlowNodeChanged(uid);
                }
                else if (itemIsType(item, FlowItemType::FTPort))
                {
                    d->m_connection = new FlowConnection();
                    scene()->addItem(d->m_connection);
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
                    if (itemIsType(item, FlowItemType::FTNode))
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
                    if (itemIsType(item, FlowItemType::FTPort))
                    {
                        auto port1 = d->m_connection->port1();
                        auto port2 = dynamic_cast<FlowPort*>(item);
                        if (port1->parentItem() != port2->parentItem() && !port1->isConnectedTo(port2))
                        {
                            d->m_connection->setPort2(port2);
                            d->m_connection->setSelected(false);
                            flag = true;
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
        if (itemIsType(item, FlowItemType::FTNode))
        {
            auto data = itemData(item, FlowItemType::FTNode).toString();
            if (data == uid) return dynamic_cast<FlowNode*>(item);
        }
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

QVariant FlowView::itemData(QGraphicsItem* item, int type)
{
    QVariant data;
    if (!itemIsType(item, type)) return data;

    switch (type)
    {
        case FlowItemType::FTNode:
            {
                auto node = dynamic_cast<FlowNode*>(item);
                if (node) return node->data(Qt::UserRole);
            }
    }
    return data;
}
