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
    static const NodeInfoList& systemNodes();

    static File create();
    static int load(File& file, const QString& path);
    static int save(const File& file, const QString& path);

public:
    void setPath(const QString& path);
    void setFile(const File& file);
    void setNode(const NodeInfo& node);
    void setVars(const VariableList& vars);
    bool save();
    QString path() const;
    File& file() const;
    NodeInfo& node() const;
    VariableList& vars() const;

signals:
    void nodeSwitched(const QString& uid, bool updateNavi);
    void nodeAdded(const QString& uid, QSharedPointer<NodeInfo> node);
    void connectionAdded(const QString& uid, const QString& connection);
    void nodePostionChanged(const QString& uid, const QPointF& pos);
    void nodeValueChanged(const NodeInfo& nodeInfo);

private:
    struct Private;
    QScopedPointer<Private> d;
};

#define DM_INST DataManager::instance()