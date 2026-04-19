#ifndef _VALUE_H_
#define _VALUE_H_

#include <QString>
#include <variant>

// a value is what flows through the flowchart at runtime
// can be a number, string, bool, or nil

struct Value{
    using Nil = std::monostate;
    using Number = double;
    using Bool = bool;
    using Str = QString;

    std::variant<Nil, Number, Bool, Str> data;

    // constructors
    Value()                 : data(Nil{}) {}
    Value(double v)         : data(v) {}
    Value(bool v)           : data(v) {}
    Value(const QString &v) : data(v) {}
    Value(const char * v)   : data(QString(v)) {}

    // type checks
    bool isNil() const{ return std::holds_alternative<Nil>(data); }
    bool isNumber() const{ return std::holds_alternative<Number>(data); }
    bool isBool() const{ return std::holds_alternative<Bool>(data); }
    bool isString() const{ return std::holds_alternative<Str>(data); }

    // accessors
    double asNumber() const { return std::get<Number>(data); }
    bool asBool() const { return std::get<Bool>(data); }
    QString asString() const { return std::get<Str>(data); }

    // nil and false are falsy, everything else truthy XD
    bool isTruthy() const{
        if(isNil()) return false;
        if(isBool()) return asBool();
        return true;
    }

    // human-readable string for output nodes
    QString toString() const{
        if(isNil()) return "nil";
        if(isNumber()){
            double n = asNumber();
            // show integers without decimal point
            if(n == static_cast<long long>(n)){
                return QString::number(static_cast<long long>(n));
            }
            return QString::number(n);
        }
        if(isBool()) return asBool() ? "true" : "false";
        if(isString()) return asString();
        return "?";
    }

    // equality
    bool operator==(const Value &other) const { return data == other.data; }
    bool operator!=(const Value &other) const { return !(*this == other); }
};

#endif
