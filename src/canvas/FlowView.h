#ifndef _FLOW_VIEW_H_
#define _FLOW_VIEW_H_


#include <QGraphicsView>

class FlowView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit FlowView(QGraphicsScene *scene, QWidget *parent = nullptr);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    bool   m_panning = false;
    QPoint m_lastPanPoint;
};

#endif
