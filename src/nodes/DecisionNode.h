#ifndef _DECISION_NODE_H_
#define _DECISION_NODE_H_

#include "FlowNode.h"

class DecisionNode : public FlowNode{
public:
    explicit DecisionNode(QGraphicsItem *parent = nullptr);

    // decision nodes ave two output ports (YES and NO)
    QPointF outputPortYes() const;
    QPointF outputPortNo() const;

    // override to allow 2 outgoing connnections
    int maxOutputConnections() const override{return 2;}

    QRectF boundingRect() const override;

protected:
    void paintShape(QPainter *painter, bool selected) override;

    // Override paint to draw the extra Yes port
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
};

#endif
