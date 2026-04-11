#ifndef _FLOW_VIEW_H_
#define _FLOW_VIEW_H_

#include <QGraphicsView>

class FlowScene;

class FlowView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit FlowView(FlowScene *scene, QWidget *parent = nullptr);

    FlowScene *flowScene() const{return m_flowScene;}

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    // show crosshair cursor when in placement mode
    void enterEvent(QEnterEvent *event) override;

private:
    FlowScene *m_flowScene;
    bool   m_panning = false;
    QPoint m_lastPanPoint;
};

#endif
