#include "FlowScene.h"
#include "FlowNode.h"
#include "StartStopNode.h"
#include "ProcessNode.h"
#include "DecisionNode.h"
#include "IONode.h"

#include <QGraphicsSceneMouseEvent>

FlowScene::FlowScene(QObject *parent)
    : QGraphicsScene(parent)
{
    setSceneRect(-2000, -2000, 4000, 4000);
    setBackgroundBrush(QColor(30, 30, 30)); // dark canvas
}

void  FlowScene::setPlacementMode(FlowNode::NodeType type){
    m_placing = true;
    m_pendingType = type;
}

void FlowScene::clearPlacementMode(){
    m_placing = false;
}

void FlowScene::mousePressEvent(QGraphicsSceneMouseEvent *event){
    if(m_placing && event->button() == Qt::LeftButton){
        createNode(m_pendingType, event->scenePos());
        clearPlacementMode();
        event->accept();
        return;
    }
    QGraphicsScene::mousePressEvent(event);
}

FlowNode* FlowScene::createNode(FlowNode::NodeType type, const QPointF &pos){
    FlowNode *node = nullptr;

    switch(type){
    case FlowNode::NodeType::StartStop:
        node = new StartStopNode(StartStopNode::Mode::Start);
        break;
    case FlowNode::NodeType::Process:
        node = new ProcessNode();
        break;
    case FlowNode::NodeType::Decision:
        node = new DecisionNode();
        break;
    case FlowNode::NodeType::IO:
        node = new IONode(IONode::Mode::Output);
        break;
    }

    if(node){
        addItem(node);
        node->setPos(pos);
    }

    return node;
}
