#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_

#include "Value.h"
#include <QString>
#include <QHash>
#include <QSharedPointer>
#include <stdexcept>

// environment stores variables by name
class Environment{
public:
    // root environment (no parent)
    Environment() : m_parent(nullptr) {}

    // child scope
    explicit Environment(QSharedPointer<Environment> parent)
        : m_parent(parent){}

    // define a new variable in the current scope
    void define(const QString &name, const Value &value){
        m_values[name] = value;
    }

    // get a variable
    Value get(const QString &name) const{
        if(m_values.contains(name))
            return m_values[name];

        if(m_parent)
            return m_parent->get(name);

        throw std::runtime_error(
            QString("Undefined variable '%1'.").arg(name).toStdString());
    }

    // assign to an existing variable
    void assign(const QString &name, const Value &value){
        if(m_values.contains(name)){
            m_values[name] = value;
            return;
        }

        if(m_parent){
            m_parent->assign(name, value);
            return;
        }

        throw std::runtime_error(
            QString("Undefined variable '%1'.").arg(name).toStdString());
    }

    // check if a variable exists anywhere in the scope chain
    bool contains(const QString &name) const{
        if(m_values.contains(name)) return true;
        if(m_parent) return m_parent->contains(name);
        return false;
    }

    // clear all variables in curent scope
    void clear() { m_values.clear();}

private:
    QHash<QString, Value> m_values;
    QSharedPointer<Environment> m_parent;
};

#endif
