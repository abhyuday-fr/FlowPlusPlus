#ifndef _PROCESS_NODE_H
#define _PROCESS_NODE_H

#include "FlowNode.h"

class ProcessNode : public FlowNode{
public:
    explicit ProcessNode(QGraphicsItem *parent = nullptr);

protected:
    void paintShape(QPainter *painter, bool selected) override;
};

#endif
