#include "FlowNode.h"
#include "FlowConnection.h"
#include "FlowScene.h"
#include "UndoCommands.h"

#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QInputDialog>
#include <QLineEdit>

FlowNode::FlowNode(NodeType type, QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , m_type(type)
    , m_label("Node")
{
    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);
}

FlowNode::~FlowNode() {}

void FlowNode::setLabel(const QString &label)
{
    m_label = label;
    update();
}

QPointF FlowNode::inputPort() const
{
    // top-center of bounding rect, in scene coords
    return mapToScene(QPointF(nodeRect().center().x(), nodeRect().top()));
}

QPointF FlowNode::outputPort() const
{
    // bottom-center of bounding rect, in scene coords
    return mapToScene(QPointF(nodeRect().center().x(), nodeRect().bottom()));
}

int FlowNode::outputConnectionCount() const{
    int count = 0;
    for(FlowConnection *conn : m_connections){
        if(conn->fromNode() ==  this){
            ++count;
        }
    }
    return count;
}

void FlowNode::addConnection(FlowConnection *conn)
{
    if (!m_connections.contains(conn)){
        m_connections.append(conn);
    }
}

void FlowNode::removeConnection(FlowConnection *conn)
{
    m_connections.removeAll(conn);
}

QRectF FlowNode::boundingRect() const
{
    // slightly larger than nodeRect to include port circles
    return nodeRect().adjusted(-PORT_RADIUS, -PORT_RADIUS, PORT_RADIUS, PORT_RADIUS);
}

QRectF FlowNode::nodeRect() const
{
    return QRectF(-NODE_WIDTH / 2, -NODE_HEIGHT / 2, NODE_WIDTH, NODE_HEIGHT);
}

void FlowNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    bool sel = isSelected();

    // let the subclass draw its shape
    paintShape(painter, sel);

    // draw label centered
    painter->setPen(Qt::white);
    QFont f = painter->font();
    f.setPointSize(9);
    painter->setFont(f);
    painter->drawText(nodeRect(), Qt::AlignCenter | Qt::TextWordWrap, m_label);

    // draw input port (top)
    painter->setBrush(QColor(100, 200, 100));
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(QPointF(nodeRect().center().x(), nodeRect().top()),PORT_RADIUS, PORT_RADIUS);

    // draw output port (bottom)
    painter->setBrush(QColor(200, 100, 100));
    painter->drawEllipse(QPointF(nodeRect().center().x(), nodeRect().bottom()),PORT_RADIUS, PORT_RADIUS);

    // highlight error with red outline
    if(m_hasError){
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(QColor(255, 60, 60), 3, Qt::DashLine));
        painter->drawRect(nodeRect());
    }
}

QVariant FlowNode::itemChange(GraphicsItemChange change, const QVariant &value){
    if (change == ItemPositionChange) {
        m_posBeforeMove = pos();
        m_moving = true;
    }

    if (change == ItemPositionHasChanged) {
        // notify all connected arrows to redraw
        for (FlowConnection *conn : m_connections)
            conn->updatePath();
    }
    return QGraphicsItem::itemChange(change, value);
}

void FlowNode::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
}

void FlowNode::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    // use the event's widget as parent to avoid nullptr parent crash
    QWidget *parentWidget = event->widget();
    QString oldLabel = m_label;

    // double-click to rename, like editing a variable name
    bool ok;
    QString text = QInputDialog::getText(parentWidget, "Edit Node Label", "Enter Label:", QLineEdit::Normal, m_label, &ok);
    if (ok && !text.isEmpty() && text != oldLabel) {
        // push to undo stack
        FlowScene *fs = dynamic_cast<FlowScene*>(scene());
        if (fs) {
            setLabel(text);
            fs->undoStack()->push(
                new EditLabelCommand(this, oldLabel, text));
        } else {
            setLabel(text);
        }
    }

    QGraphicsItem::mouseDoubleClickEvent(event);
}

void FlowNode::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    QGraphicsItem::mouseReleaseEvent(event);

    // only push if actually moved
    if (m_moving && pos() != m_posBeforeMove) {
        FlowScene *fs = dynamic_cast<FlowScene*>(scene());
        if (fs) {
            fs->undoStack()->push(
                new MoveNodeCommand(this, m_posBeforeMove, pos()));
        }
        m_moving = false;
    }
}
