#include "UndoCommands.h"
#include "FlowScene.h"
#include "FlowNode.h"
#include "FlowConnection.h"
#include "StartStopNode.h"
#include "ProcessNode.h"
#include "DecisionNode.h"
#include "IONode.h"

#include <QJsonObject>

// Helpers

QJsonObject nodeToJson(FlowNode *node, int id)
{
    QJsonObject obj;
    obj["id"] = id;
    obj["label"] = node->label();
    obj["x"] = node->pos().x();
    obj["y"] = node->pos().y();

    switch (node->nodeType()) {
    case FlowNode::NodeType::StartStop: {
        obj["type"] = QString("StartStop");
        StartStopNode *ss = dynamic_cast<StartStopNode*>(node);
        obj["mode"] = (ss && ss->mode() == StartStopNode::Mode::Start)
                          ? QString("Start") : QString("Stop");
        break;
    }
    case FlowNode::NodeType::Process:
        obj["type"] = QString("Process");
        break;
    case FlowNode::NodeType::Decision:
        obj["type"] = QString("Decision");
        break;
    case FlowNode::NodeType::IO: {
        obj["type"] = QString("IO");
        IONode *io  = dynamic_cast<IONode*>(node);
        obj["mode"] = (io && io->mode() == IONode::Mode::Input)
                          ? QString("Input") : QString("Output");
        break;
    }
    }
    return obj;
}

FlowNode* nodeFromJson(FlowScene *scene, const QJsonObject &obj)
{
    QString type = obj["type"].toString();
    QString mode = obj["mode"].toString();
    QString lbl  = obj["label"].toString();
    qreal x = obj["x"].toDouble();
    qreal y = obj["y"].toDouble();

    FlowNode *node = nullptr;

    if (type == "StartStop") {
        auto m = (mode == "Start") ? StartStopNode::Mode::Start : StartStopNode::Mode::Stop;
        node = new StartStopNode(m);
    } else if (type == "Process") {
        node = new ProcessNode();
    } else if (type == "Decision") {
        node = new DecisionNode();
    } else if (type == "IO") {
        auto m = (mode == "Input") ? IONode::Mode::Input : IONode::Mode::Output;
        node = new IONode(m);
    }

    if (node) {
        node->setLabel(lbl);
        node->setPos(x, y);
        scene->addItem(node);
    }
    return node;
}

// PlaceNodeCommand

PlaceNodeCommand::PlaceNodeCommand(FlowScene    *scene,
                                   FlowNode     *node,
                                   QUndoCommand *parent)
    : QUndoCommand("Place Node", parent)
    , m_scene(scene)
    , m_node(node)
    , m_data(nodeToJson(node))
    , m_owned(false)
{}

PlaceNodeCommand::~PlaceNodeCommand(){
    if (m_owned) delete m_node;
}

void PlaceNodeCommand::undo(){
    m_scene->removeItem(m_node);
    m_owned = true;
}

void PlaceNodeCommand::redo(){
    if (m_owned) {
        m_node->setPos(m_data["x"].toDouble(),
                       m_data["y"].toDouble());
        m_scene->addItem(m_node);
        m_owned = false;
    }
}

// DeleteCommand

DeleteCommand::DeleteCommand(FlowScene                   *scene,
                             const QList<FlowNode*>      &nodes,
                             const QSet<FlowConnection*> &conns,
                             QUndoCommand                *parent)
    : QUndoCommand("Delete", parent)
    , m_scene(scene)
{
    // Assign stable ids to nodes
    QHash<FlowNode*, int> ids;
    int id = 0;
    for (FlowNode *n : nodes) {
        ids[n] = id;
        m_nodeData << nodeToJson(n, id++);
    }

    // Also capture connections attached to selected nodes
    // that weren't directly selected
    QSet<FlowConnection*> allConns = conns;
    for (FlowNode *n : nodes)
        for (FlowConnection *c : n->connections())
            allConns.insert(c);

    for (FlowConnection *conn : allConns) {
        if (!conn->fromNode() || !conn->toNode()) continue;
        ConnectionData cd;
        // Use -1 for nodes not in our delete set
        int fromId = ids.value(conn->fromNode(), -1);
        int toId   = ids.value(conn->toNode(),   -1);
        cd.fromData = nodeToJson(conn->fromNode(), fromId);
        cd.toData   = nodeToJson(conn->toNode(),   toId);
        cd.isYes    = conn->isYesConnection();
        m_connData  << cd;
    }
}

DeleteCommand::~DeleteCommand() {}

void DeleteCommand::redo()
{
    // Find and delete matching connections first
    QList<FlowConnection*> connsToKill;
    for (QGraphicsItem *item : m_scene->items()) {
        FlowConnection *conn = dynamic_cast<FlowConnection*>(item);
        if (!conn) continue;
        // Check if this connection's endpoints match any saved conn data
        for (const ConnectionData &cd : m_connData) {
            FlowNode *from = conn->fromNode();
            FlowNode *to   = conn->toNode();
            if (!from || !to) continue;
            if (qFuzzyCompare(from->pos().x(), cd.fromData["x"].toDouble()) &&
                qFuzzyCompare(from->pos().y(), cd.fromData["y"].toDouble()) &&
                qFuzzyCompare(to->pos().x(),   cd.toData["x"].toDouble())   &&
                qFuzzyCompare(to->pos().y(),   cd.toData["y"].toDouble()))
            {
                connsToKill << conn;
                break;
            }
        }
    }
    for (FlowConnection *c : connsToKill) {
        c->detach();
        m_scene->removeItem(c);
        delete c;
    }

    // Find and delete matching nodes
    QList<FlowNode*> nodesToKill;
    for (QGraphicsItem *item : m_scene->items()) {
        FlowNode *node = dynamic_cast<FlowNode*>(item);
        if (!node) continue;
        for (const QJsonObject &obj : m_nodeData) {
            if (qFuzzyCompare(node->pos().x(), obj["x"].toDouble()) &&
                qFuzzyCompare(node->pos().y(), obj["y"].toDouble()) &&
                node->label() == obj["label"].toString())
            {
                nodesToKill << node;
                break;
            }
        }
    }
    for (FlowNode *n : nodesToKill) {
        m_scene->removeItem(n);
        delete n;
    }
}

void DeleteCommand::undo()
{
    // Recreate nodes
    QHash<int, FlowNode*> idToNode;
    int id = 0;
    for (const QJsonObject &obj : m_nodeData) {
        FlowNode *node = nodeFromJson(m_scene, obj);
        if (node) idToNode[id] = node;
        id++;
    }

    // Recreate connections
    for (const ConnectionData &cd : m_connData) {
        int fromId = cd.fromData["id"].toInt();
        int toId   = cd.toData["id"].toInt();

        FlowNode *from = idToNode.value(fromId, nullptr);
        FlowNode *to   = idToNode.value(toId,   nullptr);

        // If either node wasn't in our delete set,
        // find it in the scene by position
        if (!from) {
            for (QGraphicsItem *item : m_scene->items()) {
                FlowNode *n = dynamic_cast<FlowNode*>(item);
                if (!n) continue;
                if (qFuzzyCompare(n->pos().x(), cd.fromData["x"].toDouble()) &&
                    qFuzzyCompare(n->pos().y(), cd.fromData["y"].toDouble()))
                { from = n; break; }
            }
        }
        if (!to) {
            for (QGraphicsItem *item : m_scene->items()) {
                FlowNode *n = dynamic_cast<FlowNode*>(item);
                if (!n) continue;
                if (qFuzzyCompare(n->pos().x(), cd.toData["x"].toDouble()) &&
                    qFuzzyCompare(n->pos().y(), cd.toData["y"].toDouble()))
                { to = n; break; }
            }
        }

        if (!from || !to) continue;

        QPointF portPos;
        DecisionNode *dn = dynamic_cast<DecisionNode*>(from);
        if (dn)
            portPos = cd.isYes ? dn->outputPortYes() : dn->outputPortNo();
        else
            portPos = from->outputPort();

        FlowConnection *conn = new FlowConnection(from, to, portPos);
        m_scene->addItem(conn);
    }
}

// MoveNodeCommand

MoveNodeCommand::MoveNodeCommand(FlowNode       *node,
                                 const QPointF  &oldPos,
                                 const QPointF  &newPos,
                                 QUndoCommand   *parent)
    : QUndoCommand("Move Node", parent)
    , m_node(node)
    , m_oldPos(oldPos)
    , m_newPos(newPos)
{}

void MoveNodeCommand::undo(){
    m_node->setPos(m_oldPos);
}

void MoveNodeCommand::redo(){
    m_node->setPos(m_newPos);
}

bool MoveNodeCommand::mergeWith(const QUndoCommand *other){
    const MoveNodeCommand *o = dynamic_cast<const MoveNodeCommand*>(other);
    if (!o || o->m_node != m_node) return false;
    m_newPos = o->m_newPos;  // extend to latest position
    return true;
}

// AddConnectionCommand

AddConnectionCommand::AddConnectionCommand(FlowScene      *scene,
                                           FlowConnection *conn,
                                           QUndoCommand   *parent)
    : QUndoCommand("Add Connection", parent)
    , m_scene(scene)
    , m_conn(conn)
    , m_fromData(nodeToJson(conn->fromNode()))
    , m_toData(nodeToJson(conn->toNode()))
    , m_isYes(conn->isYesConnection())
    , m_owned(false)
{}

AddConnectionCommand::~AddConnectionCommand(){
    if (m_owned) delete m_conn;
}

void AddConnectionCommand::undo(){
    m_conn->detach();
    m_scene->removeItem(m_conn);
    m_owned = true;
}

void AddConnectionCommand::redo(){
    if (m_owned) {
        // Find the from/to nodes by position+label
        FlowNode *from = nullptr, *to = nullptr;
        for (QGraphicsItem *item : m_scene->items()) {
            FlowNode *n = dynamic_cast<FlowNode*>(item);
            if (!n) continue;
            if (qFuzzyCompare(n->pos().x(), m_fromData["x"].toDouble()) &&
                qFuzzyCompare(n->pos().y(), m_fromData["y"].toDouble()))
                from = n;
            if (qFuzzyCompare(n->pos().x(), m_toData["x"].toDouble()) &&
                qFuzzyCompare(n->pos().y(), m_toData["y"].toDouble()))
                to = n;
        }
        if (!from || !to) return;

        QPointF portPos;
        DecisionNode *dn = dynamic_cast<DecisionNode*>(from);
        if (dn)
            portPos = m_isYes ? dn->outputPortYes() : dn->outputPortNo();
        else
            portPos = from->outputPort();

        m_conn = new FlowConnection(from, to, portPos);
        m_scene->addItem(m_conn);
        m_owned = false;
    }
}

// EditLabelCommand

EditLabelCommand::EditLabelCommand(FlowNode      *node,
                                   const QString &oldLabel,
                                   const QString &newLabel,
                                   QUndoCommand  *parent)
    : QUndoCommand("Edit Label", parent)
    , m_node(node)
    , m_oldLabel(oldLabel)
    , m_newLabel(newLabel)
{}

void EditLabelCommand::undo(){
    m_node->setLabel(m_oldLabel);
}

void EditLabelCommand::redo(){
    m_node->setLabel(m_newLabel);
}

bool EditLabelCommand::mergeWith(const QUndoCommand *other){
    const EditLabelCommand *o =
        dynamic_cast<const EditLabelCommand*>(other);
    if (!o || o->m_node != m_node) return false;
    m_newLabel = o->m_newLabel;
    return true;
}
