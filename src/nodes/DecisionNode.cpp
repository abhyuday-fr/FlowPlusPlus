#include "DecisionNode.h"
#include "FlowConnection.h"

#include <QPainter>
#include <QPen>
#include <QColor>
#include <QPolygonF>

DecisionNode::DecisionNode(QGraphicsItem *parent)
    : FlowNode(NodeType::Decision, parent)
{
    setLabel("Decision?");
}

QPointF DecisionNode::outputPortYes() const{
    // right=center of the diamond (yes branch)
    return mapToScene(QPointF(nodeRect().right(), nodeRect().center().y()));
}

QPointF DecisionNode::outputPortNo() const{
    return outputPort();
}

void DecisionNode::paintShape(QPainter *painter, bool selected){
    QRectF r = nodeRect();
    QColor fill(140, 90, 20);
    QColor border = selected ? QColor(255, 220, 0) : fill.lighter(150);

    // diamon.. what is it? just the 4 midpoints of the bounding rect loll
    QPolygonF diamond;
    diamond << QPointF(r.center().x(), r.top()) //top
            << QPointF(r.right(), r.center().y()) //right
            << QPointF(r.center().x(), r.bottom()) //bottom
            << QPointF(r.left(), r.center().y());

    painter->setBrush(fill);
    painter->setPen(QPen(border, selected ? 2.5 : 1.5));
    painter->drawPolygon(diamond);
}

void DecisionNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    // draw the base (shape + label + input + No port)
    FlowNode::paint(painter, option, widget);

    // draw yes port, right center (green)
    painter->setBrush(QColor(80, 200, 80));
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(
        QPointF(nodeRect().right(), nodeRect().center().y()),
        PORT_RADIUS, PORT_RADIUS);

    // Port labels
    painter->setPen(Qt::white);
    QFont f = painter->font();
    f.setPointSize(7);
    painter->setFont(f);
    painter->drawText(QPointF(nodeRect().right() + PORT_RADIUS + 2, nodeRect().center().y() + 4), "Yes");
    painter->drawText(QPointF(nodeRect().center().x() + PORT_RADIUS + 2, nodeRect().bottom() + 12), "No");
}

QRectF DecisionNode::boundingRect() const
{
    // extend right to cover the Yes port
    return nodeRect().adjusted(
        -(NODE_WIDTH / 2 + PORT_RADIUS),        // left, left diamond tip
        -PORT_RADIUS,                            // top
        NODE_WIDTH / 2 + PORT_RADIUS + 30,      // right, Yes port + "Yes" text
        PORT_RADIUS + 16                         // bottom, No label below
    );
}
