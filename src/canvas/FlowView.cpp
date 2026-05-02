#include "FlowView.h"
#include "FlowScene.h"
#include "MiniMap.h"

#include <QWheelEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QEnterEvent>
#include <QResizeEvent>

void FlowView::initMiniMap(FlowScene *scene)
{
    m_miniMap = new MiniMap(scene, this, this);
    repositionMiniMap();
    m_miniMap->show();
}

void FlowView::repositionMiniMap()
{
    if (!m_miniMap) return;
    // Bottom-right corner with a small margin
    int margin = 12;
    m_miniMap->move(
        width()  - m_miniMap->width()  - margin,
        height() - m_miniMap->height() - margin
        );
}

void FlowView::scrollContentsBy(int dx, int dy)
{
    QGraphicsView::scrollContentsBy(dx, dy);
    if (m_miniMap) m_miniMap->update();
}

void FlowView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    repositionMiniMap();
    if (m_miniMap) m_miniMap->update();
}

FlowView::FlowView(FlowScene *scene, QWidget *parent)
    : QGraphicsView(scene, parent),
      m_flowScene(scene)
{
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::NoDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setFrameShape(QFrame::NoFrame);
    setFocusPolicy(Qt::StrongFocus);
}

void FlowView::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        // Ctrl+Scroll = zoom
        if (event->angleDelta().y() > 0) zoomIn();
        else zoomOut();
        event->accept();
        return;
    }
    // Normal scroll = pan vertically
    QGraphicsView::wheelEvent(event);
}

void FlowView::mousePressEvent(QMouseEvent *event)
{
    setFocus(); // clicking anywhere on the canvass reclaims focus
    if (event->button() == Qt::MiddleButton) {
        m_panning = true;
        m_lastPanPoint = event->pos();
        setCursor(Qt::ClosedHandCursor);
    }
    QGraphicsView::mousePressEvent(event);
}

void FlowView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_panning) {
        QPoint delta = event->pos() - m_lastPanPoint;
        m_lastPanPoint = event->pos();
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
    }
    QGraphicsView::mouseMoveEvent(event);
}

void FlowView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        m_panning = false;
        setCursor(Qt::ArrowCursor);
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void FlowView::enterEvent(QEnterEvent *event){
    if(m_flowScene && m_flowScene->isPlacing())
        setCursor(Qt::CrossCursor);
    else
        setCursor(Qt::ArrowCursor);
    QGraphicsView::enterEvent(event);
}

void FlowView::zoomIn(){
    if (m_zoomLevel >= MAX_ZOOM) return;
    m_zoomLevel = qMin(m_zoomLevel * 1.2, MAX_ZOOM);
    setTransform(QTransform::fromScale(m_zoomLevel, m_zoomLevel));
}

void FlowView::zoomOut(){
    if (m_zoomLevel <= MIN_ZOOM) return;
    m_zoomLevel = qMax(m_zoomLevel / 1.2, MIN_ZOOM);
    setTransform(QTransform::fromScale(m_zoomLevel, m_zoomLevel));
}

void FlowView::zoomReset(){
    m_zoomLevel = 1.0;
    setTransform(QTransform::fromScale(1.0, 1.0));
}
