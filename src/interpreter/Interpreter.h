#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_

#include "Value.h"
#include "Environment.h"
#include <QString>
#include <QStringList>
#include <QSharedPointer>
#include <QList>

class FlowNode;
class FlowScene;

//-------
// Token
//-------

struct Token{
    enum class Type{
        // literals
        Number, String, True, False, Nil,

        // identifiers
        Identifier,

        // operators
        Plus, Minus, Star, Slash, Percent,
        EqualEqual, BangEqual,
        Less, LessEqual, Greater, GreaterEqual,
        Bang, And, Or,
        Equal,

        // delimeters
        LeftParen, RightParen, Comma,

        // keywords
        Print, Input,

        // end
        Eof
    };

    Type type;
    QString lexeme;
    Value literal; // for number/string tokens
};

//-----------------------------------------------------
// Lexer: converts the content into tokens (tokenizer)
//-----------------------------------------------------

class Lexer{
public:
    explicit Lexer(const QString &source);
    QList<Token> tokenize();

private:
    QString m_source;
    int m_start = 0;
    int m_current = 0;

    bool isAtEnd() const { return m_current >= m_source.size(); }
    QChar advance() { return m_source[m_current++]; }
    QChar peek() const { return isAtEnd() ? '\0' : m_source[m_current]; }
    QChar peekNext() const{
        return (m_current + 1 >= m_source.size()) ? '\0' : m_source[m_current + 1];
    }

    bool match(QChar expected);
    Token makeToken(Token::Type type);
    Token makeToken(Token::Type type, const Value &literal);
    Token string();
    Token number();
    Token identifier();
};

//---------------------------------------------------------
// Parser: converts tokens into AST (Abstract Syntax Tree)
//---------------------------------------------------------

// we represent expressions as a simple recursive structure
struct Expr{
    enum class Kind{
        Literal,    // 42, "hello", true
        Variable,   // x
        Assign,     // x = expr
        Unary,      // -x, !x
        Binary,     // x + y, x > y
        Logical,    // x and y, x or y
        Print,      // print expr
        Input,      // input varname
    };

    Kind kind;
    Value value;    // for Literal
    QString name;   // for Variable, Assign, Input
    QString op;     // for Binary/Unary/Logical
    QSharedPointer<Expr> left;
    QSharedPointer<Expr> right;
};

class Parser{
public:
    explicit Parser(const QList<Token> &tokens);
    QSharedPointer<Expr> parse();

private:
    QList<Token> m_tokens;
    int m_current = 0;

    Token peek() const { return m_tokens[m_current]; }
    Token previous() const { return m_tokens[m_current - 1]; }
    bool isAtEnd() const { return peek().type == Token::Type::Eof; }
    Token advance();
    bool  check(Token::Type type) const;
    bool  match(std::initializer_list<Token::Type> types);
    Token consume(Token::Type type, const QString &msg);

    QSharedPointer<Expr> expression();
    QSharedPointer<Expr> assignment();
    QSharedPointer<Expr> logicalOr();
    QSharedPointer<Expr> logicalAnd();
    QSharedPointer<Expr> equality();
    QSharedPointer<Expr> comparison();
    QSharedPointer<Expr> term();
    QSharedPointer<Expr> factor();
    QSharedPointer<Expr> unary();
    QSharedPointer<Expr> primary();
};

//------------
//Interpreter
//------------

class Interpreter{
public:
    explicit Interpreter(FlowScene *scene);

    // run the flowchart, returns output lines
    QStringList run();

private:
    FlowScene *m_scene;
    QSharedPointer<Environment> m_env;
    QStringList m_output;

    // graph traversal
    FlowNode* findStartNode();
    FlowNode* nextNode(FlowNode *current, bool &isYesbranch);

    // expression execution
    Value evaluate(const QSharedPointer<Expr> &expr);
    Value executeNode(FlowNode *node);

    // helpers
    Value evalBinary(const QString &op, const Value &l, const Value &r);
    QString requestInput(const QString &prompt);

    int m_stepLimit = 1000; // prevent infinite loops
    void setStepLimit(int limit) { m_stepLimit  = limit; }

    QList<FlowNode*> m_errorNodes;
};

#endif
