#ifndef MINIMAP_H
#define MINIMAP_H

#include <QWidget>

class FlowScene;
class FlowView;

class MiniMap : public QWidget
{
    Q_OBJECT

public:
    explicit MiniMap(FlowScene *scene, FlowView *view, QWidget *parent = nullptr);

    void updateMap();

protected:
    void paintEvent(QPaintEvent *event)  override;
    void mousePressEvent(QMouseEvent *event)   override;
    void mouseMoveEvent(QMouseEvent *event)    override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    FlowScene *m_scene;
    FlowView  *m_view;

    bool   m_dragging = false;

    // Convert between minimap coords and scene coords
    QRectF  sceneRect() const;
    QRectF  viewportRect() const;
    QPointF toScene(const QPointF &miniPos) const;
    QRectF  toMini(const QRectF  &sceneR) const;

    static constexpr int   MAP_W = 200;
    static constexpr int   MAP_H = 150;
    static constexpr int   MARGIN = 12;
    static constexpr float OPACITY = 0.85f;
};

#endif // MINIMAP_H
