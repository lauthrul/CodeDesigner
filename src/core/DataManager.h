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
    static FunctionList loadFunctionsFromFile(const QString& path);
    static const FunctionList& systemFunctions();
    static bool isSystemFunction(const QString& funcName);

    static NodeInfo create();
    static NodeInfo load(const QString& path);
    static bool save(const NodeInfo& node);
    using  traverseNodeInfoFunc = std::function<bool(NodeInfo* node, NodeInfo* root, void* userData)>;
    static bool traverseNodeInfo(NodeInfo* node, NodeInfo* root, traverseNodeInfoFunc func, void* userData = nullptr);

public:
    void setFilePath(const QString& path);
    void setRootNode(const NodeInfo& root);
    QString filePath() const;
    NodeInfo& rootNode() const;

signals:
    void nodeAdded(QSharedPointer<NodeInfo> node);

private:
    struct Private;
    QScopedPointer<Private> d;
};
