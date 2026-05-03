#include "UndoCommands.h"
#include "FlowScene.h"
#include "FlowNode.h"
#include "FlowConnection.h"
#include "StartStopNode.h"
#include "ProcessNode.h"
#include "DecisionNode.h"
#include "IONode.h"

#include <QJsonObject>

// Helpers (Keep these for copy/paste or save/load if needed elsewhere, 
// though they are no longer used by UndoCommands for ownership)

// PlaceNodeCommand

PlaceNodeCommand::PlaceNodeCommand(FlowScene    *scene,
                                   FlowNode     *node,
                                   QUndoCommand *parent)
    : QUndoCommand("Place Node", parent)
    , m_scene(scene)
    , m_node(node)
    , m_owned(false)
{}

PlaceNodeCommand::~PlaceNodeCommand(){
    if (m_owned) delete m_node;
}

void PlaceNodeCommand::undo(){
    m_scene->removeItem(m_node);
    m_owned = true;
}

void PlaceNodeCommand::redo(){
    m_scene->addItem(m_node);
    m_owned = false;
}

// DeleteCommand

DeleteCommand::DeleteCommand(FlowScene                   *scene,
                             const QList<FlowNode*>      &nodes,
                             const QSet<FlowConnection*> &conns,
                             QUndoCommand                *parent)
    : QUndoCommand("Delete", parent)
    , m_scene(scene)
    , m_nodes(nodes)
    , m_conns(conns.values())
    , m_owned(false)
{}

DeleteCommand::~DeleteCommand() {
    if (m_owned) {
        for (FlowConnection *c : m_conns) delete c;
        for (FlowNode *n : m_nodes) delete n;
    }
}

void DeleteCommand::redo()
{
    for (FlowConnection *c : m_conns) {
        c->detach();
        m_scene->removeItem(c);
    }
    for (FlowNode *n : m_nodes) {
        m_scene->removeItem(n);
    }
    m_owned = true;
}

void DeleteCommand::undo()
{
    for (FlowNode *n : m_nodes) {
        m_scene->addItem(n);
    }
    for (FlowConnection *c : m_conns) {
        c->attach(); // need to re-attach to nodes
        m_scene->addItem(c);
    }
    m_owned = false;
}

// MoveNodeCommand

MoveNodeCommand::MoveNodeCommand(FlowNode       *node,
                                 const QPointF  &oldPos,
                                 const QPointF  &newPos,
                                 QUndoCommand   *parent)
    : QUndoCommand("Move Node", parent)
    , m_node(node)
    , m_oldPos(oldPos)
    , m_newPos(newPos)
{}

void MoveNodeCommand::undo(){
    m_node->setPos(m_oldPos);
}

void MoveNodeCommand::redo(){
    m_node->setPos(m_newPos);
}

bool MoveNodeCommand::mergeWith(const QUndoCommand *other){
    const MoveNodeCommand *o = dynamic_cast<const MoveNodeCommand*>(other);
    if (!o || o->m_node != m_node) return false;
    m_newPos = o->m_newPos;
    return true;
}

// AddConnectionCommand

AddConnectionCommand::AddConnectionCommand(FlowScene      *scene,
                                           FlowConnection *conn,
                                           QUndoCommand   *parent)
    : QUndoCommand("Add Connection", parent)
    , m_scene(scene)
    , m_conn(conn)
    , m_owned(false)
{}

AddConnectionCommand::~AddConnectionCommand(){
    if (m_owned) delete m_conn;
}

void AddConnectionCommand::undo(){
    m_conn->detach();
    m_scene->removeItem(m_conn);
    m_owned = true;
}

void AddConnectionCommand::redo(){
    m_conn->attach();
    m_scene->addItem(m_conn);
    m_owned = false;
}

// EditLabelCommand

EditLabelCommand::EditLabelCommand(FlowNode      *node,
                                   const QString &oldLabel,
                                   const QString &newLabel,
                                   QUndoCommand  *parent)
    : QUndoCommand("Edit Label", parent)
    , m_node(node)
    , m_oldLabel(oldLabel)
    , m_newLabel(newLabel)
{}

void EditLabelCommand::undo(){
    m_node->setLabel(m_oldLabel);
}

void EditLabelCommand::redo(){
    m_node->setLabel(m_newLabel);
}

bool EditLabelCommand::mergeWith(const QUndoCommand *other){
    const EditLabelCommand *o =
        dynamic_cast<const EditLabelCommand*>(other);
    if (!o || o->m_node != m_node) return false;
    m_newLabel = o->m_newLabel;
    return true;
}
