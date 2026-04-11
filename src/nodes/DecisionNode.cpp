#include "DecisionNode.h"
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
