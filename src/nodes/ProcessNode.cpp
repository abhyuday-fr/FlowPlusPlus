#include "ProcessNode.h"
#include <QPainter>
#include <QPen>
#include <QColor>

ProcessNode::ProcessNode(QGraphicsItem *parent)
    : FlowNode(NodeType::Process, parent)
{
    setLabel("Process");
}

void ProcessNode::paintShape(QPainter *painter, bool selected)
{
    QColor fill(50, 90, 160);
    QColor border = selected ? QColor(255, 220, 0) : fill.lighter(140);

    painter->setBrush(fill);
    painter->setPen(QPen(border, selected ? 2.5 : 1.5));
    painter->drawRect(nodeRect());
}
