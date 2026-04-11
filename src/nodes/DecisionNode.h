#ifndef _DECISION_NODE_H_
#define _DECISION_NODE_H_

#include "FlowNode.h"

class DecisionNode : public FlowNode{
public:
    explicit DecisionNode(QGraphicsItem *parent = nullptr);

    // decision nodes ave two output ports (YES and NO)
    QPointF outputPortYes() const;
    QPointF outputPortNo() const;

protected:
    void paintShape(QPainter *painter, bool selected) override;
};

#endif
