#include "FlowConnection.h"
#include "FlowNode.h"
#include "DecisionNode.h"

#include <QPen>
#include <QPainterPath>
#include <cmath>

static qreal ptDist(const QPointF &a, const QPointF &b){
    QPointF d = a - b;
    return std::sqrt(d.x()*d.x() + d.y()*d.y());
}

FlowConnection::FlowConnection(FlowNode *from, FlowNode *to, const QPointF &startPortPos, QGraphicsItem *parent)
    : QGraphicsPathItem(parent)
    , m_from(from)
    , m_to(to)
    , m_fromYesPort(false)
{
    // Decide at construction time, never recalculate
    DecisionNode *dn = dynamic_cast<DecisionNode*>(from);
    if (dn) {
        qreal distYes = ptDist(startPortPos, dn->outputPortYes());
        qreal distNo  = ptDist(startPortPos, dn->outputPortNo());
        m_fromYesPort = (distYes < distNo);
    }

    setPen(QPen(QColor(180, 180, 255), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    setZValue(-1); // draw behind nodes

    setFlag(QGraphicsItem::ItemIsSelectable, true);

    from->addConnection(this);
    to->addConnection(this);

    updatePath();
}

FlowConnection::~FlowConnection()
{
    if (m_from) m_from->removeConnection(this);
    if (m_to)   m_to->removeConnection(this);
}

void FlowConnection::updatePath()
{
    if (!m_from || !m_to) return;

    // always use the remembered port
    QPointF start;
    DecisionNode *dn = dynamic_cast<DecisionNode*>(m_from);
    if (dn)
        start = m_fromYesPort ? dn->outputPortYes() : dn->outputPortNo();
    else
        start = m_from->outputPort();

    QPointF end = m_to->inputPort();

    // cubic bezier i.e smooth curved arrow
    QPainterPath path(start);
    qreal dx = (end.x() - start.x()) * 0.5;
    qreal dy = (end.y() - start.y()) * 0.5;
    path.cubicTo(start + QPointF(dx * 0.2, dy), end - QPointF(dx * 0.2, dy), end);

    // highlight if selected
    if(isSelected())
        setPen(QPen(QColor(255, 220, 0), 2.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    else
        setPen(QPen(QColor(180, 180, 255), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    setPath(path);
}

void FlowConnection::detach(){
    if (m_from) { m_from->removeConnection(this); m_from = nullptr; }
    if (m_to)   { m_to->removeConnection(this);   m_to   = nullptr; }
}
