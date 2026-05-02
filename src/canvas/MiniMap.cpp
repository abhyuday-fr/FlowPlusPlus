#include "MiniMap.h"
#include "FlowScene.h"
#include "FlowView.h"

#include <QPainter>
#include <QMouseEvent>
#include <QGraphicsItem>
#include <typeinfo>

MiniMap::MiniMap(FlowScene *scene, FlowView *view, QWidget *parent)
    : QWidget(parent)
    , m_scene(scene)
    , m_view(view)
{
    setFixedSize(MAP_W, MAP_H);
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);

    connect(scene, &FlowScene::changed,
            this,  [this](const QList<QRectF>&){ update(); });
}

// coordinate helpers

QRectF MiniMap::sceneRect() const
{
    QRectF r = m_scene->itemsBoundingRect();
    if (r.isEmpty())
        r = QRectF(-400, -400, 800, 800);
    r.adjust(-80, -80, 80, 80);
    return r;
}

QRectF MiniMap::toMini(const QRectF &sceneR) const
{
    QRectF sr = sceneRect();
    if (sr.width() <= 0 || sr.height() <= 0) return {};

    // leave 1px border inside the widget
    const qreal pw = MAP_W - 2;
    const qreal ph = MAP_H - 2;

    qreal sx = pw / sr.width();
    qreal sy = ph / sr.height();

    return QRectF(
        1 + (sceneR.x() - sr.x()) * sx,
        1 + (sceneR.y() - sr.y()) * sy,
        sceneR.width()  * sx,
        sceneR.height() * sy
        );
}

QPointF MiniMap::toScene(const QPointF &miniPos) const
{
    QRectF sr = sceneRect();
    const qreal pw = MAP_W - 2;
    const qreal ph = MAP_H - 2;
    return QPointF(
        sr.x() + (miniPos.x() - 1) * sr.width()  / pw,
        sr.y() + (miniPos.y() - 1) * sr.height() / ph
        );
}

QRectF MiniMap::viewportRect() const
{
    // get the visible scene area from the view
    QRectF vp = m_view->mapToScene(
                          m_view->viewport()->rect())
                    .boundingRect();
    return toMini(vp);
}

// paint

void MiniMap::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // background panel
    p.setBrush(QColor(18, 18, 18, 220));
    p.setPen(QPen(QColor(70, 70, 70), 1));
    p.drawRoundedRect(rect().adjusted(0,0,-1,-1), 8, 8);

    // clip drawing to inside the panel
    QPainterPath clip;
    clip.addRoundedRect(rect().adjusted(1,1,-1,-1), 7, 7);
    p.setClipPath(clip);

    // draw connections first (behind nodes)
    for (QGraphicsItem *item : m_scene->items()) {
        QString tn = typeid(*item).name();
        if (!tn.contains("Connection")) continue;

        QRectF miniRect = toMini(
            item->mapToScene(item->boundingRect()).boundingRect());

        p.setBrush(QColor(180, 180, 255, 80));
        p.setPen(QPen(QColor(180, 180, 255, 150), 1));
        p.drawLine(miniRect.topLeft(), miniRect.bottomRight());
    }

    // draw nodes
    for (QGraphicsItem *item : m_scene->items()) {
        QString tn = typeid(*item).name();
        QColor fill;

        if      (tn.contains("StartStop"))  fill = QColor(60,  140, 80);
        else if (tn.contains("Process"))    fill = QColor(50,  90,  160);
        else if (tn.contains("Decision"))   fill = QColor(140, 90,  20);
        else if (tn.contains("IONode"))     fill = QColor(30,  120, 140);
        else continue;

        QRectF miniRect = toMini(
            item->mapToScene(item->boundingRect()).boundingRect());

        // Minimum visible size
        if (miniRect.width()  < 4) miniRect.setWidth(4);
        if (miniRect.height() < 3) miniRect.setHeight(3);

        p.setBrush(fill);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(miniRect, 2, 2);
    }

    // viewport rectangle — yellow outline showing visible area
    p.setClipping(false);
    QRectF vp = viewportRect();
    if (!vp.isEmpty()) {
        p.setBrush(QColor(255, 220, 0, 25));
        p.setPen(QPen(QColor(255, 220, 0), 1.5));
        p.drawRect(vp);
    }

    // label
    p.setPen(QColor(120, 120, 120));
    p.setFont(QFont("Sans", 7));
    p.drawText(rect().adjusted(4, 2, -4, -2),
               Qt::AlignTop | Qt::AlignRight, "overview");
}

// mouse

void MiniMap::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_view->centerOn(toScene(event->pos()));
        update();
    }
}

void MiniMap::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging) {
        m_view->centerOn(toScene(event->pos()));
        update();
    }
}

void MiniMap::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        m_dragging = false;
}
