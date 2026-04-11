#include "StartStopNode.h"
#include <QPainter>
#include <QPen>
#include <QColor>

StartStopNode::StartStopNode(Mode mode, QGraphicsItem *parent)
    : FlowNode(NodeType::StartStop, parent),
    m_mode(mode)
{
    setLabel(mode == Mode::Start ? "Start" : "Stop");
}

void StartStopNode::paintShape(QPainter *painter, bool selected){
    //green for Start and red for Stop
    QColor fill = (m_mode == Mode::Start) ? QColor(60, 140, 80) : QColor(160, 60, 60);
    QColor border = selected ? QColor(255, 255, 0) : fill.lighter(130);

    painter->setBrush(fill);
    painter->setPen(QPen(border, selected ? 2.5 : 1.5));

    // pill/stadium shape (fully rounded ends)
    painter->drawRoundedRect(nodeRect(), NODE_HEIGHT / 2, NODE_HEIGHT / 2);
}
