#ifndef _IO_NODE_H_
#define _IO_NODE_H_

#include "FlowNode.h"

class IONode : public FlowNode{
public:
    enum class Mode {Input, Output};

    explicit IONode(Mode mode, QGraphicsItem *parent = nullptr);

    Mode mode() const{ return m_mode;}

    QRectF boundingRect() const override;

protected:
    void paintShape(QPainter *painter, bool selected) override;

private:
    Mode m_mode;
};

#endif
