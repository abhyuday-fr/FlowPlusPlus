#ifndef _FLOW_CONNECTION_H
#define FLOW_CONNECTION_H

#include <QGraphicsPathItem>
#include <QPointF>

class FlowNode;

class FlowConnection : public QGraphicsPathItem
{
public:
    explicit FlowConnection(FlowNode *from, FlowNode *to, const QPointF &startPortPos ,QGraphicsItem *parent = nullptr);
    ~FlowConnection() override;

    FlowNode *fromNode() const { return m_from; }
    FlowNode *toNode()   const { return m_to;   }

    // called whenever a connected node moves
    void updatePath();

    bool isYesConnection() const { return m_fromYesPort; }

    void detach();
    void attach();

private:
    FlowNode *m_from;
    FlowNode *m_to;
    bool m_fromYesPort;
};

#endif
