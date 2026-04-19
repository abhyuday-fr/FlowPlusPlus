#include "FlowScene.h"
#include "FlowConnection.h"
#include "FlowNode.h"
#include "StartStopNode.h"
#include "ProcessNode.h"
#include "DecisionNode.h"
#include "IONode.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsPathItem>
#include <QKeyEvent>
#include <QPen>
#include <cmath>

static constexpr qreal PORT_HIT_RADIUS = 12.0;

FlowScene::FlowScene(QObject *parent)
    : QGraphicsScene(parent)
{
    setSceneRect(-2000, -2000, 4000, 4000);
    setBackgroundBrush(QColor(30, 30, 30)); // dark canvas
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

void FlowScene::keyPressEvent(QKeyEvent *event){
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        const QList<QGraphicsItem*> selected = selectedItems();
        for (QGraphicsItem *item : selected) {
            // ff a connection, just remove it
            if (FlowConnection *conn = dynamic_cast<FlowConnection*>(item)) {
                removeItem(conn);
                delete conn;
                continue;
            }
            // if a node, remove its connections first then the node
            if (FlowNode *node = dynamic_cast<FlowNode*>(item)) {
                // copy list, removing modifies it
                QList<FlowConnection*> conns = node->connections();
                for (FlowConnection *conn : conns) {
                    conn->detach();
                    removeItem(conn);
                    delete conn;
                }
                removeItem(node);
                delete node;
            }
        }
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
