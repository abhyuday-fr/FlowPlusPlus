#ifndef _FLOW_NODE_H
#define _FLOW_NODE_H

#include <QGraphicsItem>
#include <QString>
#include <QList>

class FlowConnection;

class FlowNode : public QGraphicsItem
{
public:
    // node types (like token types)
    enum class NodeType {
        StartStop,
        Process,
        Decision,
        IO
    };

    explicit FlowNode(NodeType type, QGraphicsItem *parent = nullptr);
    ~FlowNode() override;

    // every node must know its type
    NodeType nodeType() const { return m_type; }

    // label shown inside the shape
    QString label() const        { return m_label; }
    void    setLabel(const QString &label);

    // conneection ports (where arrows attach)
    // returns scene-space positions of input/output ports
    virtual QPointF inputPort()  const;
    virtual QPointF outputPort() const;

    // how many outgoing connections are allowed
    virtual int maxOutputConnections() const { return 1; }

    // current outgoing connection count
    int outputConnectionCount() const;

    // track connections for graph traversal
    void addConnection(FlowConnection *conn);
    void removeConnection(FlowConnection *conn);
    const QList<FlowConnection*>& connections() const { return m_connections; }

    // QGraphicsItem interface
    QRectF boundingRect() const override;
    void paint(QPainter *painter,const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    // subclasses define their own shape geometry
    virtual QRectF  nodeRect()  const;
    virtual void    paintShape(QPainter *painter, bool selected) = 0;

    // allow dragging
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

    NodeType             m_type;
    QString              m_label;
    QList<FlowConnection*> m_connections;

    // visual constants (shared by all nodes)
    static constexpr qreal NODE_WIDTH  = 160.0;
    static constexpr qreal NODE_HEIGHT =  60.0;
    static constexpr qreal PORT_RADIUS =   5.0;
};

#endif
