#pragma once
#include "Models.h"
#include <QObject>

class DataManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(DataManager)

public:
    DataManager(QObject* parent = nullptr);
    ~DataManager();

public:
    static DataManager* instance();
    static QString genUUid();

    static FunctionList loadFunctions(const QString& path);
    static const FunctionList& systemFunctions();
    static bool isSystemFunction(const QString& funcName);

    static NodeInfo createNodeInfo();
    static NodeInfo loadNodeInfo(const QString& path);
    static bool saveNodeInfo(const NodeInfo& node, const QString& path);
    using  traverseNodeInfoFunc = std::function<bool(NodeInfo* node, NodeInfo* root, void* userData)>;
    static bool traverseNodeInfo(NodeInfo* node, NodeInfo* root, traverseNodeInfoFunc func, void* userData = nullptr);

public:
    void setCurrentFilePath(const QString& path);
    void setCurrentRootNode(const NodeInfo& root);
    QString currentFilePath() const;
    NodeInfo& currentRootNode() const;
    bool saveCurrentNodeInfo();

signals:
    void nodeSwitched(const QString& uid, bool updateNavi);
    void nodeAdded(QSharedPointer<NodeInfo> node);

private:
    struct Private;
    QScopedPointer<Private> d;
};

#define DM_INST DataManager::instance()