#ifndef _FLOW_SCENE_H_
#define _FLOW_SCENE_H_

#include <QGraphicsScene>

class FlowScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit FlowScene(QObject *parent = nullptr);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
};

#endif
