#ifndef _FLOW_SCENE_H_
#define _FLOW_SCENE_H_

#include <QGraphicsScene>
#include "FlowNode.h"

class FlowScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit FlowScene(QObject *parent = nullptr);

    // called by toolbar buttons to arm a node type for placement
    void setPlacementMode(FlowNode::NodeType type);
    void clearPlacementMode();
    bool isPlacing() const{ return m_placing;}

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    FlowNode* createNode(FlowNode::NodeType type, const QPointF &pos);

    bool m_placing = false;
    FlowNode::NodeType m_pendingType;
};

#endif
