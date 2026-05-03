#ifndef UNDOCOMMANDS_H
#define UNDOCOMMANDS_H

#include <QUndoCommand>
#include <QPointF>
#include <QString>
#include <QJsonObject>
#include <QSet>

class FlowScene;
class FlowNode;
class FlowConnection;
class StartStopNode;
class IONode;

// --------
// Helpers
//---------

// place Node

class PlaceNodeCommand : public QUndoCommand{
public:
    PlaceNodeCommand(FlowScene    *scene,
                     FlowNode     *node,
                     QUndoCommand *parent = nullptr);
    ~PlaceNodeCommand() override;

    void undo() override;
    void redo() override;

private:
    FlowScene   *m_scene;
    FlowNode    *m_node;
    bool         m_owned;  // true when removed from scene
};

// delete

class DeleteCommand : public QUndoCommand{
public:
    DeleteCommand(FlowScene *scene,
                  const QList<FlowNode*> &nodes,
                  const QSet<FlowConnection*>&conns,
                  QUndoCommand *parent = nullptr);
    ~DeleteCommand() override;

    void undo() override;
    void redo() override;

private:
    FlowScene              *m_scene;
    QList<FlowNode*>        m_nodes;
    QList<FlowConnection*>  m_conns;
    bool                    m_owned; // true when removed from scene
};

// move Node

class MoveNodeCommand : public QUndoCommand
{
public:
    MoveNodeCommand(FlowNode       *node,
                    const QPointF  &oldPos,
                    const QPointF  &newPos,
                    QUndoCommand   *parent = nullptr);

    void undo() override;
    void redo() override;

    // merge consecutive moves of same node into one command
    bool mergeWith(const QUndoCommand *other) override;
    int  id() const override { return 1001; }

private:
    FlowNode *m_node;
    QPointF   m_oldPos;
    QPointF   m_newPos;
};

// add connection

class AddConnectionCommand : public QUndoCommand{
public:
    AddConnectionCommand(FlowScene      *scene,
                         FlowConnection *conn,
                         QUndoCommand   *parent = nullptr);
    ~AddConnectionCommand() override;

    void undo() override;
    void redo() override;

private:
    FlowScene *m_scene;
    FlowConnection *m_conn;
    bool m_owned;
};

// edit label
class EditLabelCommand : public QUndoCommand{
public:
    EditLabelCommand(FlowNode      *node,
                     const QString &oldLabel,
                     const QString &newLabel,
                     QUndoCommand  *parent = nullptr);

    void undo() override;
    void redo() override;

    bool mergeWith(const QUndoCommand *other) override;
    int  id() const override { return 1002; }

private:
    FlowNode *m_node;
    QString m_oldLabel;
    QString m_newLabel;
};

#endif // UNDOCOMMANDS_H
