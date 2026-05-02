#include "FlowScene.h"
#include "FlowConnection.h"
#include "FlowNode.h"
#include "StartStopNode.h"
#include "ProcessNode.h"
#include "DecisionNode.h"
#include "IONode.h"
#include "UndoCommands.h"

#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsPathItem>
#include <QKeyEvent>
#include <QPen>
#include <cmath>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QIODevice>
#include <QUndoStack>

static constexpr qreal PORT_HIT_RADIUS = 12.0;

FlowScene::FlowScene(QObject *parent)
    : QGraphicsScene(parent)
    , m_undoStack(new QUndoStack(this))
{
    setSceneRect(-2000, -2000, 4000, 4000);
    setBackgroundBrush(QColor(30, 30, 30)); // dark canvas
}

bool FlowScene::isNearOutputPort(const QPointF &scenePos){
    return outputPortAt(scenePos).node != nullptr;
}

// copy/pase
void FlowScene::copySelected(){
    const QList<QGraphicsItem*> selected = selectedItems();
    if (selected.isEmpty()) return;

    // build a map of selected nodes only
    QHash<FlowNode*, int> nodeIds;
    QJsonArray nodesArr;
    int id = 0;

    for (QGraphicsItem *item : selected) {
        FlowNode *node = dynamic_cast<FlowNode*>(item);
        if (!node) continue;
        nodeIds[node] = id;

        QJsonObject obj;
        obj["id"]    = id++;
        obj["label"] = node->label();
        obj["x"]     = node->pos().x();
        obj["y"]     = node->pos().y();

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
        nodesArr.append(obj);
    }

    // only copy connections where BOTH endpoints are selected
    QJsonArray connsArr;
    for (QGraphicsItem *item : selected) {
        FlowConnection *conn = dynamic_cast<FlowConnection*>(item);
        if (!conn) continue;
        if (!nodeIds.contains(conn->fromNode())) continue;
        if (!nodeIds.contains(conn->toNode()))   continue;

        QJsonObject obj;
        obj["from"]  = nodeIds[conn->fromNode()];
        obj["to"]    = nodeIds[conn->toNode()];
        obj["isYes"] = conn->isYesConnection();
        connsArr.append(obj);
    }

    // also copy connections between selected nodes
    // even if the connection itself isn't selected
    for (QGraphicsItem *item : selected) {
        FlowNode *node = dynamic_cast<FlowNode*>(item);
        if (!node) continue;
        for (FlowConnection *conn : node->connections()) {
            if (!nodeIds.contains(conn->fromNode())) continue;
            if (!nodeIds.contains(conn->toNode()))   continue;

            // Avoid duplicates
            bool already = false;
            for (const QJsonValue &v : connsArr) {
                QJsonObject o = v.toObject();
                if (o["from"].toInt() == nodeIds[conn->fromNode()] &&
                    o["to"].toInt()   == nodeIds[conn->toNode()])
                { already = true; break; }
            }
            if (!already) {
                QJsonObject obj;
                obj["from"]  = nodeIds[conn->fromNode()];
                obj["to"]    = nodeIds[conn->toNode()];
                obj["isYes"] = conn->isYesConnection();
                connsArr.append(obj);
            }
        }
    }

    m_clipboard = QJsonArray();
    QJsonObject data;
    data["nodes"]       = nodesArr;
    data["connections"] = connsArr;
    m_clipboard.append(data);
}

void FlowScene::pasteClipboard(){
    if (m_clipboard.isEmpty()) return;

    QJsonObject data  = m_clipboard.first().toObject();
    QJsonArray  nodes = data["nodes"].toArray();
    QJsonArray  conns = data["connections"].toArray();

    // paste offset so nodes don't land exactly on top of originals
    constexpr qreal OFFSET = 30.0;

    // deselect everything first
    clearSelection();

    QHash<int, FlowNode*> idToNode;

    for (const QJsonValue &val : nodes) {
        QJsonObject obj  = val.toObject();
        int         id   = obj["id"].toInt();
        QString     type = obj["type"].toString();
        QString     mode = obj["mode"].toString();
        QString     lbl  = obj["label"].toString();
        qreal       x    = obj["x"].toDouble() + OFFSET;
        qreal       y    = obj["y"].toDouble() + OFFSET;

        FlowNode *node = nullptr;

        if (type == "StartStop") {
            auto m = (mode == "Start")
            ? StartStopNode::Mode::Start
            : StartStopNode::Mode::Stop;
            node = new StartStopNode(m);
        } else if (type == "Process") {
            node = new ProcessNode();
        } else if (type == "Decision") {
            node = new DecisionNode();
        } else if (type == "IO") {
            auto m = (mode == "Input")
            ? IONode::Mode::Input
            : IONode::Mode::Output;
            node = new IONode(m);
        }

        if (!node) continue;

        node->setLabel(lbl);
        node->setPos(x, y);
        addItem(node);
        node->setSelected(true);  // select pasted nodes
        idToNode[id] = node;
    }

    // rebuild connections between pasted nodes
    for (const QJsonValue &val : conns) {
        QJsonObject obj    = val.toObject();
        int         fromId = obj["from"].toInt();
        int         toId   = obj["to"].toInt();
        bool        isYes  = obj["isYes"].toBool();

        if (!idToNode.contains(fromId)) continue;
        if (!idToNode.contains(toId))   continue;

        FlowNode *from = idToNode[fromId];
        FlowNode *to   = idToNode[toId];

        QPointF portPos;
        DecisionNode *dn = dynamic_cast<DecisionNode*>(from);
        if (dn)
            portPos = isYes ? dn->outputPortYes() : dn->outputPortNo();
        else
            portPos = from->outputPort();

        FlowConnection *conn = new FlowConnection(from, to, portPos);
        addItem(conn);
    }
}

void  FlowScene::setPlacementMode(FlowNode::NodeType type,
                                 StartStopNode::Mode ssMode,
                                 bool ioInput){
    m_placing = true;
    m_pendingType = type;
    m_pendingSSMode = ssMode;
    m_pendingIOInput = ioInput;
}

void FlowScene::clearPlacementMode(){
    m_placing = false;
}

//---------------
// Port Detection
//----------------

static qreal dist(const QPointF &a, const QPointF &b){
    QPointF d = a - b;
    return std::sqrt(d.x() * d.x() + d.y() * d.y());
}

FlowScene::PortHit FlowScene::outputPortAt(const QPointF &scenePos){
    for (QGraphicsItem *item : items()) {
        FlowNode *node = dynamic_cast<FlowNode*>(item);
        if (!node) continue;

        // main output port (bottom / No)
        if (dist(scenePos, node->outputPort()) <= PORT_HIT_RADIUS)
            return { node, node->outputPort(), false };

        // yes port, Decision only
        DecisionNode *dn = dynamic_cast<DecisionNode*>(node);
        if (dn && dist(scenePos, dn->outputPortYes()) <= PORT_HIT_RADIUS)
            return { dn, dn->outputPortYes(), true };
    }
    return {};
}

FlowNode* FlowScene::inputPortAt(const QPointF &scenePos){
    for (QGraphicsItem *item : items()) {
        FlowNode *node = dynamic_cast<FlowNode*>(item);
        if (!node) continue;

        if (dist(scenePos, node->inputPort()) <= PORT_HIT_RADIUS){
            // count existing incoming connections
            int incoming = 0;
            for(FlowConnection *conn : node->connections()){
                if(conn->toNode() == node){
                    ++incoming;
                }
            }

            if(incoming < node->maxInputConnections())
                return node;
        }

    }
    return nullptr;
}

//--------------
// Mouse Events
//--------------

void FlowScene::mousePressEvent(QGraphicsSceneMouseEvent *event){
    // grab keyboard focus so Delete/Backspace work
    if (!views().isEmpty())
        views().first()->setFocus();

    if(m_placing && event->button() == Qt::LeftButton){
        createNode(m_pendingType, event->scenePos());
        clearPlacementMode();
        event->accept();
        return;
    }

    //Check if user clicked near an output port, start connecting
    if(event->button() == Qt::LeftButton){
        PortHit hit = outputPortAt(event->scenePos());
        if (hit.node) {
            // Check output limit
            if (hit.node->outputConnectionCount() >= hit.node->maxOutputConnections()) {
                event->accept();
                return;
            }

            m_connecting  = true;
            m_connFrom    = hit.node;
            m_connFromPos = hit.portPos;

            // temporary dashed line to show drag
            QPainterPath path(m_connFromPos);
            path.lineTo(event->scenePos());
            m_tempLine = addPath(path, QPen(QColor(200, 200, 255), 1.5, Qt::DashLine));
            event->accept();
            return;
        }
    }
    QGraphicsScene::mousePressEvent(event);
}

void FlowScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
    if(m_connecting && m_tempLine && m_connFrom){
        QPainterPath path(m_connFromPos);
        path.lineTo(event->scenePos());
        m_tempLine->setPath(path);
        event->accept();
        return;
    }
    QGraphicsScene::mouseMoveEvent(event);
}

void FlowScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    if(m_connecting){
        // check if we released over an input port
        FlowNode *target = inputPortAt(event->scenePos());

        if (target && target != m_connFrom) {
            FlowConnection *conn = new FlowConnection(
                m_connFrom, target, m_connFromPos);
            addItem(conn);
            m_undoStack->push(new AddConnectionCommand(this, conn));
        }

        // clean up temp line
        if(m_tempLine){
            removeItem(m_tempLine);
            delete m_tempLine;
            m_tempLine = nullptr;
        }

        m_connecting = false;
        m_connFrom = nullptr;
        event->accept();
        return;
    }
    QGraphicsScene::mouseReleaseEvent(event);
}

//------------------------
// Delete Selected Items
//------------------------

void FlowScene::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_C && event->modifiers() & Qt::ControlModifier) {
        copySelected();
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_V && event->modifiers() & Qt::ControlModifier) {
        pasteClipboard();
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_A && event->modifiers() & Qt::ControlModifier) {
        for (QGraphicsItem *item : items())
            item->setSelected(true);
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace){
        const QList<QGraphicsItem*> selected = selectedItems();
        if (selected.isEmpty()) {
            event->accept();
            return;
        }

        QSet<FlowConnection*> connsToDelete;
        QList<FlowNode*>      nodesToDelete;

        for (QGraphicsItem *item : selected) {
            if (FlowConnection *conn = dynamic_cast<FlowConnection*>(item))
                connsToDelete.insert(conn);
            if (FlowNode *node = dynamic_cast<FlowNode*>(item)) {
                nodesToDelete.append(node);
                for (FlowConnection *conn : node->connections())
                    connsToDelete.insert(conn);
            }
        }

        // Let the command handle the actual deletion
        m_undoStack->push(new DeleteCommand(this, nodesToDelete,
                                            connsToDelete));

        // Refocus
        QGraphicsView *v = views().isEmpty() ? nullptr : views().first();
        if (v && v->isVisible())
            v->setFocus();

        event->accept();
        return;
    }

    QGraphicsScene::keyPressEvent(event);
}

//-------------
// Node factory
//-------------

FlowNode* FlowScene::createNode(FlowNode::NodeType type, const QPointF &pos){
    FlowNode *node = nullptr;

    switch(type){
    case FlowNode::NodeType::StartStop:
        node = new StartStopNode(m_pendingSSMode);
        break;
    case FlowNode::NodeType::Process:
        node = new ProcessNode();
        break;
    case FlowNode::NodeType::Decision:
        node = new DecisionNode();
        break;
    case FlowNode::NodeType::IO:
        node = new IONode(m_pendingIOInput ? IONode::Mode::Input : IONode::Mode::Output);
        break;
    }

    if(node){
        addItem(node);
        node->setPos(pos);
        m_undoStack->push(new PlaceNodeCommand(this, node));
    }

    return node;
}

void FlowScene::clearAll(){
    // first detach and delete all connections
    QList<FlowConnection*> conns;
    for(QGraphicsItem *item : items()){
        if(FlowConnection *conn = dynamic_cast<FlowConnection*>(item))
            conns.append(conn);
    }
    
    for(FlowConnection *conn : conns){
        conn->detach();
        removeItem(conn);
        delete conn;
    }
    
    // then delete all nodes
    QList<FlowNode*> nodes;
    for(QGraphicsItem *item : items()){
        if(FlowNode *node = dynamic_cast<FlowNode*>(item)){
            nodes.append(node);
        }
    }

    for(FlowNode *node : nodes){
        removeItem(node);
        delete node;
    }
}

// Save

bool FlowScene::saveToFile(const QString &path){
    // build a stable id for each node
    QHash<FlowNode*, int> nodeIds;
    QJsonArray nodesArr;
    int id = 0;

    for(QGraphicsItem *item : items()){
        FlowNode *node = dynamic_cast<FlowNode*>(item);
        if(!node) continue;

        nodeIds[node] = id;

        QJsonObject obj;
        obj["id"] = id++;
        obj["label"] = node->label();
        obj["x"] = node->pos().x();
        obj["y"] = node->pos().y();

        // type string
        switch (node->nodeType()) {
        case FlowNode::NodeType::StartStop: {
        obj["type"] = QString("StartStop");
        StartStopNode *ss = dynamic_cast<StartStopNode*>(node);
        obj["mode"] = (ss && ss->mode() == StartStopNode::Mode::Start) ? QString("Start") : QString("Stop");
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
            obj["mode"] = (io && io->mode() == IONode::Mode::Input) ? QString("Input") : QString("Output");
            break;
        }
        }

        nodesArr.append(obj);
    }
    // serialize connections
    QJsonArray connsArr;
    for(QGraphicsItem *item : items()){
        FlowConnection *conn = dynamic_cast<FlowConnection*>(item);
        if(!conn) continue;
        if(!conn->fromNode() || !conn->toNode()) continue;
        if (!nodeIds.contains(conn->fromNode())) continue;
        if (!nodeIds.contains(conn->toNode())) continue;

        QJsonObject obj;
        obj["from"] = nodeIds[conn->fromNode()];
        obj["to"] = nodeIds[conn->toNode()];
        obj["isYes"] = conn->isYesConnection();
        connsArr.append(obj);
    }

    QJsonObject root;
    root["version"] = 1;
    root["nodes"] = nodesArr;
    root["connections"] = connsArr;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

// load
bool FlowScene::loadFromFile(const QString &path){
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly))
        return false;

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    if(err.error != QJsonParseError::NoError)
        return false;

    QJsonObject root = doc.object();
    if (root["version"].toInt() != 1)
        return false;

    // clear existing canvas
    clearAll();

    // rebuild nodes
    QHash<int, FlowNode*> idToNode;

    for (const QJsonValue &val : root["nodes"].toArray()) {
        QJsonObject obj  = val.toObject();
        int id = obj["id"].toInt();
        QString type = obj["type"].toString();
        QString mode = obj["mode"].toString();
        QString lbl = obj["label"].toString();
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

        if (!node) continue;

        node->setLabel(lbl);
        node->setPos(x, y);
        addItem(node);
        idToNode[id] = node;
    }

    // rebuild connections
    for (const QJsonValue &val : root["connections"].toArray()) {
        QJsonObject obj   = val.toObject();
        int fromId = obj["from"].toInt();
        int toId = obj["to"].toInt();
        bool isYes  = obj["isYes"].toBool();

        if (!idToNode.contains(fromId)) continue;
        if (!idToNode.contains(toId))   continue;

        FlowNode *from = idToNode[fromId];
        FlowNode *to = idToNode[toId];

        // Determine correct port position
        QPointF portPos;
        DecisionNode *dn = dynamic_cast<DecisionNode*>(from);
        if (dn)
            portPos = isYes ? dn->outputPortYes() : dn->outputPortNo();
        else
            portPos = from->outputPort();

        FlowConnection *conn = new FlowConnection(from, to, portPos);
        addItem(conn);
    }

    return true;
}
