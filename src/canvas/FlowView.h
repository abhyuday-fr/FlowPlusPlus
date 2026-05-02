#ifndef _FLOW_VIEW_H_
#define _FLOW_VIEW_H_

#include <QGraphicsView>
#include <MiniMap.h>

class FlowScene;

class FlowView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit FlowView(FlowScene *scene, QWidget *parent = nullptr);

    FlowScene *flowScene() const{return m_flowScene;}

    void zoomIn();
    void zoomOut();
    void zoomReset();

    static constexpr double MIN_ZOOM = 0.1;
    static constexpr double MAX_ZOOM = 4.0;

    void initMiniMap(FlowScene *scene);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    // show crosshair cursor when in placement mode
    void enterEvent(QEnterEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

    void scrollContentsBy(int dx, int dy) override;

private:
    FlowScene *m_flowScene;
    bool   m_panning = false;
    QPoint m_lastPanPoint;

    double m_zoomLevel = 1.0;

    MiniMap *m_miniMap = nullptr;
    void repositionMiniMap();
};

#endif
