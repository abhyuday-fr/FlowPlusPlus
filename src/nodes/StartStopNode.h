#ifndef _START_STOP_NODE_H_
#define _START_STOP_NODE_H_

#include "FlowNode.h"

class StartStopNode : public FlowNode{
public:
    enum class Mode {Start, Stop};

    explicit StartStopNode(Mode mode, QGraphicsItem *parent = nullptr);

    Mode mode() const {return m_mode;}

protected:
    void paintShape(QPainter *painter, bool selected) override;

private:
    Mode m_mode;
};

#endif
