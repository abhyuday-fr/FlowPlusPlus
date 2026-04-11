#ifndef _FLOW_CONNECTION_H
#define FLOW_CONNECTION_H

#include <QGraphicsPathItem>

class FlowNode;

class FlowConnection : public QGraphicsPathItem
{
public:
    explicit FlowConnection(FlowNode *from, FlowNode *to, QGraphicsItem *parent = nullptr);
    ~FlowConnection() override;

    FlowNode *fromNode() const { return m_from; }
    FlowNode *toNode()   const { return m_to;   }

    // called whenever a connected node moves
    void updatePath();

private:
    FlowNode *m_from;
    FlowNode *m_to;
};

#endif
