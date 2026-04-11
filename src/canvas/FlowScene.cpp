#include "FlowScene.h"
#include <QGraphicsSceneMouseEvent>

FlowScene::FlowScene(QObject *parent)
    : QGraphicsScene(parent)
{
    setSceneRect(-2000, -2000, 4000, 4000);
    setBackgroundBrush(QColor(30, 30, 30)); // dark canvas
}

void FlowScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mousePressEvent(event);
}
