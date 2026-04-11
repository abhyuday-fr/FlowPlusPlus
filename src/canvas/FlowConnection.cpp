#include "FlowConnection.h"
#include "FlowNode.h"

#include <QPen>
#include <QPainterPath>

FlowConnection::FlowConnection(FlowNode *from, FlowNode *to, QGraphicsItem *parent)
    : QGraphicsPathItem(parent)
    , m_from(from)
    , m_to(to)
{
    setPen(QPen(QColor(180, 180, 255), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    setZValue(-1); // draw behind nodes
    updatePath();

    from->addConnection(this);
    to->addConnection(this);
}

FlowConnection::~FlowConnection()
{
    if (m_from) m_from->removeConnection(this);
    if (m_to)   m_to->removeConnection(this);
}

void FlowConnection::updatePath()
{
    if (!m_from || !m_to) return;

    QPointF start = m_from->outputPort();
    QPointF end   = m_to->inputPort();

    // cubic bezier i.e smooth curved arrow
    QPainterPath path(start);
    qreal dy = (end.y() - start.y()) * 0.5;
    path.cubicTo(start + QPointF(0, dy), end   - QPointF(0, dy), end);

    setPath(path);
}
