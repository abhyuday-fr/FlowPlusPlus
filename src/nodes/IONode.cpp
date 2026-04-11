#include "IONode.h"
#include <QPainter>
#include <QPen>
#include <QColor>
#include <QPolygonF>

IONode::IONode(Mode mode, QGraphicsItem *parent)
    : FlowNode(NodeType::IO, parent),
    m_mode(mode)
{
    setLabel(mode == Mode::Input ? "Input" : "Output");
}

void IONode::paintShape(QPainter *painter, bool selected){
    QRectF r = nodeRect();

    //purple for input, teal for output (I thought of these colors while playing UNO Flip lol)
    QColor fill = (m_mode == Mode::Input) ? QColor(80, 60, 140) : QColor(30, 120, 140);

    QColor border = selected ? QColor(255, 110, 0) : fill.lighter(140);

    // Parallelogram..hmm.. just shift the top-left and top-right by an offset
    constexpr qreal skew = 16.0;
    QPolygonF para;
    para << QPointF(r.left() + skew, r.top())
         << QPointF(r.right() + skew, r.top())
         << QPointF(r.right() - skew, r.bottom())
         << QPointF(r.left() - skew, r.bottom());

    painter->setBrush(fill);
    painter->setPen(QPen(border, selected ? 2.5 : 1.5));
    painter->drawPolygon(para);
}

QRectF IONode::boundingRect() const
{
    // extra horizontal padding to account for parallelogram skew
    constexpr qreal skew = 16.0;
    return nodeRect().adjusted(-skew, -PORT_RADIUS, skew, PORT_RADIUS);
}
