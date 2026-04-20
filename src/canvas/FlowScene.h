#ifndef _FLOW_SCENE_H_
#define _FLOW_SCENE_H_

#include <QString>
#include <QGraphicsScene>
#include <QGraphicsPathItem>
#include "FlowNode.h"
#include "StartStopNode.h"
#include "IONode.h"

class FlowConnection;
class DecisionNode;

class FlowScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit FlowScene(QObject *parent = nullptr);

    // called by toolbar buttons to arm a node type for placement
    void setPlacementMode(FlowNode::NodeType type,
                          StartStopNode::Mode ssMode = StartStopNode::Mode::Start,
                          bool ioInput = false);
    void clearPlacementMode();
    bool isPlacing() const{ return m_placing;}

    void clearAll();

    bool saveToFile(const QString &path);
    bool loadFromFile(const QString &path);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    FlowNode* createNode(FlowNode::NodeType type, const QPointF &pos);

    // returns node + exact port position if click is near any output port
    struct PortHit{
        FlowNode *node = nullptr;
        QPointF portPos;
        bool isYes = false;
    };
    PortHit outputPortAt(const QPointF &scenePos);
    FlowNode* inputPortAt(const QPointF &scenePos);

    // placement
    bool m_placing = false;
    FlowNode::NodeType m_pendingType;

    // connection dragginmg
    bool m_connecting = false;
    FlowNode *m_connFrom = nullptr;
    QPointF m_connFromPos;
    QGraphicsPathItem *m_tempLine = nullptr;

    StartStopNode::Mode m_pendingSSMode = StartStopNode::Mode::Start;
    bool m_pendingIOInput = false;
};

#endif
