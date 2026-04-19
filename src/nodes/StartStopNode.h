#ifndef _START_STOP_NODE_H_
#define _START_STOP_NODE_H_

#include "FlowNode.h"

class StartStopNode : public FlowNode{
public:
    enum class Mode {Start, Stop};

    explicit StartStopNode(Mode mode, QGraphicsItem *parent = nullptr);

    Mode mode() const {return m_mode;}

    // start with no input port and Stop with no output port
    int maxOutputConnections() const override{
        return m_mode == Mode::Stop ? 0 : 1;
    }

    int maxInputConnections() const override{
        return m_mode == Mode::Start ? 0 : 1;
    }

protected:
    void paintShape(QPainter *painter, bool selected) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    Mode m_mode;
};

#endif
