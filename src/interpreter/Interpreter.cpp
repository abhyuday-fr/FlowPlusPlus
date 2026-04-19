#include "Interpreter.h"
#include "FlowNode.h"
#include "FlowScene.h"
#include "FlowConnection.h"
#include "DecisionNode.h"
#include "StartStopNode.h"
#include "IONode.h"

#include <QInputDialog>
#include <QApplication>
#include <stdexcept>

//=========
// Lexer
//=========

Lexer::Lexer(const QString &source) : m_source(source) {}

bool Lexer::match(QChar expected){
    if(isAtEnd() || m_source[m_current] != expected) return false;
    m_current++;
    return true;
}

Token Lexer::makeToken(Token::Type type){
    return {type, m_source.mid(m_start, m_current - m_start), Value() };
}

Token Lexer::makeToken(Token::Type type, const Value &literal){
    return {type, m_source.mid(m_start, m_current - m_start), literal };
}

Token Lexer::string(){
    while (!isAtEnd() && peek() != '"') advance();
    advance(); // closing "
    QString val = m_source.mid(m_start + 1, m_current - m_start - 2);
    return makeToken(Token::Type::String, Value(val));
}

Token Lexer::number(){
    while (peek().isDigit()) advance();
    if (peek() == '.' && peekNext().isDigit()) {
        advance();
        while (peek().isDigit()) advance();
    }
    double val = m_source.mid(m_start, m_current - m_start).toDouble();
    return makeToken(Token::Type::Number, Value(val));
}

Token Lexer::identifier(){
    while (peek().isLetterOrNumber() || peek() == '_') advance();
    QString text = m_source.mid(m_start, m_current - m_start);

    if(text == "true") return makeToken(Token::Type::True, Value(true));
    if(text == "false") return makeToken(Token::Type::False, Value(false));
    if(text == "nil") return makeToken(Token::Type::Nil, Value());
    if(text == "and") return makeToken(Token::Type::And);
    if(text == "or") return makeToken(Token::Type::Or);
    if(text == "print") return makeToken(Token::Type::Print);
    if(text == "input") return makeToken(Token::Type::Input);

    return makeToken(Token::Type::Identifier);
}

QList<Token> Lexer::tokenize(){
    QList<Token> tokens;

    while(!isAtEnd()){
        m_start = m_current;
        QChar c = advance();

        if(c.isSpace()) continue;
        if(c.isDigit()) { tokens << number(); continue;}
        if(c.isLetter() || c == '_') { tokens << identifier(); continue;}

        switch(c.toLatin1()){
        case '"': tokens << string(); break;
        case '+':  tokens << makeToken(Token::Type::Plus); break;
        case '-':  tokens << makeToken(Token::Type::Minus); break;
        case '*':  tokens << makeToken(Token::Type::Star); break;
        case '/':  tokens << makeToken(Token::Type::Slash); break;
        case '%':  tokens << makeToken(Token::Type::Percent);break;
        case '(':  tokens << makeToken(Token::Type::LeftParen); break;
        case ')':  tokens << makeToken(Token::Type::RightParen); break;
        case '!':  tokens << makeToken(match('=') ? Token::Type::BangEqual
                                    : Token::Type::Bang); break;
        case '=':  tokens << makeToken(match('=') ? Token::Type::EqualEqual
                                    : Token::Type::Equal); break;
        case '<':  tokens << makeToken(match('=') ? Token::Type::LessEqual
                                    : Token::Type::Less); break;
        case '>':  tokens << makeToken(match('=') ? Token::Type::GreaterEqual
                                    : Token::Type::Greater); break;
        default: break;
        }
    }

    tokens << Token{ Token::Type::Eof, "", Value() };
    return tokens;
}

//=========
// Parser
//=========

Parser::Parser(const QList<Token> &tokens) : m_tokens(tokens) {}

Token Parser::advance(){
    if(!isAtEnd()) m_current++;
    return previous();
}

bool Parser::check(Token::Type type) const{
    return !isAtEnd() && peek().type == type;
}

bool Parser::match(std::initializer_list<Token::Type> types){
    for(auto type : types){
        if(check(type)){
            advance();
            return true;
        }
    }
    return false;
}

Token Parser::consume(Token::Type type, const QString &msg){
    if(check(type)) return advance();
    throw std::runtime_error(msg.toStdString());
}

QSharedPointer<Expr> Parser::parse(){
    if (isAtEnd()) return nullptr;
    return expression();
}

QSharedPointer<Expr> Parser::expression() { return assignment(); }

QSharedPointer<Expr> Parser::assignment(){
    // print expr
    if(match({Token::Type::Print})){
        auto val = logicalOr();
        auto e = QSharedPointer<Expr>::create();
        e->kind = Expr::Kind::Print;
        e->right = val;
        return e;
    }

    // input varname
    if(match({Token::Type::Input})){
        consume(Token::Type::Identifier, "Expected variable name after 'input'");
        auto e = QSharedPointer<Expr>::create();
        e->kind = Expr::Kind::Input;
        e->name = previous().lexeme;
        return e;
    }

    auto expr = logicalOr();

    // x = value
    if(match({Token::Type::Equal})){
        auto val = logicalOr();
        if(expr->kind == Expr::Kind::Variable){
            auto e = QSharedPointer<Expr>::create();
            e->kind = Expr::Kind::Assign;
            e->name = expr->name;
            e->right = val;
            return e;
        }
        throw std::runtime_error("Invalid assignment target.");
    }

    return expr;
}

QSharedPointer<Expr> Parser::logicalOr(){
    auto expr = logicalAnd();
    while (match({Token::Type::Or})) {
        QString op = previous().lexeme;
        auto right = logicalAnd();
        auto e = QSharedPointer<Expr>::create();
        e->kind = Expr::Kind::Logical;
        e->op = op;
        e->left = expr;
        e->right = right;
        expr = e;
    }
    return expr;
}

QSharedPointer<Expr> Parser::logicalAnd(){
    auto expr = equality();
    while (match({Token::Type::And})) {
        QString op = previous().lexeme;
        auto right = equality();
        auto e = QSharedPointer<Expr>::create();
        e->kind = Expr::Kind::Logical;
        e->op = op;
        e->left = expr;
        e->right = right;
        expr = e;
    }
    return expr;
}

QSharedPointer<Expr> Parser::equality(){
    auto expr = comparison();
    while (match({Token::Type::EqualEqual, Token::Type::BangEqual})) {
        QString op = previous().lexeme;
        auto right = comparison();
        auto e = QSharedPointer<Expr>::create();
        e->kind = Expr::Kind::Binary;
        e->op = op;
        e->left = expr;
        e->right = right;
        expr = e;
    }
    return expr;
}

QSharedPointer<Expr> Parser::comparison(){
    auto expr = term();
    while (match({Token::Type::Less,
                  Token::Type::LessEqual,
                  Token::Type::Greater,
                  Token::Type::GreaterEqual})) {
        QString op = previous().lexeme;
        auto right = term();
        auto e = QSharedPointer<Expr>::create();
        e->kind = Expr::Kind::Binary;
        e->op = op;
        e->left = expr;
        e->right = right;
        expr = e;
    }
    return expr;
}

QSharedPointer<Expr> Parser::term(){
    auto expr = factor();
    while (match({Token::Type::Plus, Token::Type::Minus})) {
        QString op = previous().lexeme;
        auto right = factor();
        auto e = QSharedPointer<Expr>::create();
        e->kind = Expr::Kind::Binary;
        e->op = op;
        e->left = expr;
        e->right = right;
        expr = e;
    }
    return expr;
}

QSharedPointer<Expr> Parser::factor(){
    auto expr = unary();
    while (match({Token::Type::Star,
                  Token::Type::Slash,
                  Token::Type::Percent})) {
        QString op = previous().lexeme;
        auto right = unary();
        auto e = QSharedPointer<Expr>::create();
        e->kind = Expr::Kind::Binary;
        e->op = op;
        e->left = expr;
        e->right = right;
        expr = e;
    }
    return expr;
}

QSharedPointer<Expr> Parser::unary(){
    if (match({Token::Type::Bang, Token::Type::Minus})) {
        QString op = previous().lexeme;
        auto right = unary();
        auto e = QSharedPointer<Expr>::create();
        e->kind = Expr::Kind::Unary;
        e->op = op;
        e->right = right;
        return e;
    }
    return primary();
}

QSharedPointer<Expr> Parser::primary(){
    if (match({Token::Type::Number,
               Token::Type::String,
               Token::Type::True,
               Token::Type::False,
               Token::Type::Nil})) {
        auto e = QSharedPointer<Expr>::create();
        e->kind = Expr::Kind::Literal;
        e->value = previous().literal;
        return e;
    }

    if (match({Token::Type::Identifier})) {
        auto e = QSharedPointer<Expr>::create();
        e->kind = Expr::Kind::Variable;
        e->name = previous().lexeme;
        return e;
    }

    if (match({Token::Type::LeftParen})) {
        auto expr = expression();
        consume(Token::Type::RightParen, "Expected ')' after expression.");
        return expr;
    }

    throw std::runtime_error(
        QString("Unexpected token: '%1'").arg(peek().lexeme).toStdString());
}

//=============
// Interpreter
//=============

Interpreter::Interpreter(FlowScene *scene) :
    m_scene(scene), m_env(QSharedPointer<Environment>::create()) {}

//-----------------
// Graph Traversal
//----------------

FlowNode* Interpreter::findStartNode(){
    for(QGraphicsItem *item : m_scene->items()){
        StartStopNode *n = dynamic_cast<StartStopNode*>(item);
        if(n && n->mode() == StartStopNode::Mode::Start)
            return n;
    }
    return nullptr;
}

FlowNode* Interpreter::nextNode(FlowNode *current, bool &takeYes){
    // for decision nodes we need to pick the right branch
    DecisionNode *dn = dynamic_cast<DecisionNode*>(current);

    for(FlowConnection *conn : current->connections()){
        if(conn->fromNode() != current) continue;

        if(dn){
            // decison node, pick Yes or No branch
            if(takeYes && conn->isYesConnection()) return conn->toNode();
            if(!takeYes && !conn->isYesConnection()) return conn->toNode();
        }
        else{
            // every other node, jsut follow the single conncection
            return conn->toNode();
        }
    }
    return nullptr;
}

//-----------------------
// expression evaluation
//-----------------------

Value Interpreter::evaluate(const QSharedPointer<Expr> &expr){
    if(!expr) return Value();

    switch(expr->kind){
    case Expr::Kind::Literal:
        return expr->value;

    case Expr::Kind::Variable:
        return m_env->get(expr->name);

    case Expr::Kind::Assign: {
        Value val = evaluate(expr->right);
        if (m_env->contains(expr->name))
            m_env->assign(expr->name, val);
        else
            m_env->define(expr->name, val);
        return val;
    }
    case Expr::Kind::Print: {
        Value val = evaluate(expr->right);
        m_output << val.toString();
        return val;
    }

    case Expr::Kind::Input: {
        QString prompt = expr->name.isEmpty() ? "Input:" : expr->name + ":";
        QString userInput = requestInput(prompt);
        Value val;
        bool ok;
        double d = userInput.toDouble(&ok);
        if (ok) val = Value(d);
        else    val = Value(userInput);
        m_env->define(expr->name, val);
        return val;
    }

    case Expr::Kind::Unary: {
        Value right = evaluate(expr->right);
        if (expr->op == "-" && right.isNumber())
            return Value(-right.asNumber());
        if (expr->op == "!")
            return Value(!right.isTruthy());
        return Value();
    }

    case Expr::Kind::Binary:
        return evalBinary(expr->op,
                          evaluate(expr->left),
                          evaluate(expr->right));

    case Expr::Kind::Logical: {
        Value left = evaluate(expr->left);
        if (expr->op == "or")
            return left.isTruthy() ? left : evaluate(expr->right);
        // and
        return !left.isTruthy() ? left : evaluate(expr->right);
    }

    }

    return Value();
}

Value Interpreter::evalBinary(const QString &op,const Value  &l, const Value  &r){
    if (op == "+" ) {
        if (l.isNumber() && r.isNumber())
            return Value(l.asNumber() + r.asNumber());
        return Value(l.toString() + r.toString()); // string concat
    }
    if (op == "-"  && l.isNumber() && r.isNumber())
        return Value(l.asNumber() - r.asNumber());
    if (op == "*"  && l.isNumber() && r.isNumber())
        return Value(l.asNumber() * r.asNumber());
    if (op == "/"  && l.isNumber() && r.isNumber())
        return Value(r.asNumber() != 0 ? l.asNumber() / r.asNumber() : 0.0);
    if (op == "%"  && l.isNumber() && r.isNumber())
        return Value(std::fmod(l.asNumber(), r.asNumber()));
    if (op == "==" ) return Value(l == r);
    if (op == "!=" ) return Value(l != r);
    if (op == "<"  && l.isNumber() && r.isNumber())
        return Value(l.asNumber() <  r.asNumber());
    if (op == "<=" && l.isNumber() && r.isNumber())
        return Value(l.asNumber() <= r.asNumber());
    if (op == ">"  && l.isNumber() && r.isNumber())
        return Value(l.asNumber() >  r.asNumber());
    if (op == ">=" && l.isNumber() && r.isNumber())
        return Value(l.asNumber() >= r.asNumber());
    return Value();
}

Value Interpreter::executeNode(FlowNode *node){

    // Start and Stop nodes have no expression to evaluate so skip them
    StartStopNode *ss = dynamic_cast<StartStopNode*>(node);
    if(ss) return Value();

    QString label = node->label().trimmed();
    if (label.isEmpty()) return Value();

    // IO nodes: prefix determines behavior
    IONode *io = dynamic_cast<IONode*>(node);
    if (io) {
        if (io->mode() == IONode::Mode::Output) {
            // treat label as expression to print
            label = "print " + label;
        } else {
            // treat label as variable name to read into
            label = "input " + label;
        }
    }

    try {
        Lexer  lexer(label);
        Parser parser(lexer.tokenize());
        auto   expr = parser.parse();
        if (!expr) return Value();
        return evaluate(expr);
    } catch (const std::exception &e) {
        m_output << QString("[Error in '%1': %2]").arg(node->label(),
                            QString::fromStdString(e.what()));
        node->setError(true);
        m_errorNodes.append(node);
        return Value();
    }
}

QString Interpreter::requestInput(const QString &prompt){
    bool ok;
    QString result = QInputDialog::getText(
        nullptr, "FlowPlusPlus Input", prompt,
        QLineEdit::Normal, "", &ok);
    return ok ? result : "";
}

//---------------
// main run loop
//----------------

QStringList Interpreter::run(){
    // clear previous error highlights
    for(FlowNode *n : m_errorNodes){
        n->setError(false);
        n->update();
    }
    m_errorNodes.clear();

    m_output.clear();
    m_env->clear();

    FlowNode *current = findStartNode();
    if(!current){
        m_output << "[Error: No Start node found]";
        return m_output;
    }

    // m_output << "[Debug: Found Start node]";

    int steps = 0;

    while(current){
        // stop node : work's done.. phew
        StartStopNode *ss = dynamic_cast<StartStopNode*>(current);
        if(ss && ss->mode() == StartStopNode::Mode::Stop){
            // m_output << "[Debug: Reached Stop]";
            break;
        }

        if(++steps > m_stepLimit){
            m_output << QString("[Error: Execution limit of %1 steps reached."
                                " Possible infinite loop.]").arg(m_stepLimit);

            break;
        }

        // m_output << "[Debug: Visiting node: " + current->label() + "]";

        // execute the node
        Value result = executeNode(current);

        // determine next code (follow the graph)
        DecisionNode *dn = dynamic_cast<DecisionNode*>(current);
        if(dn){
            bool yes = result.isTruthy();
            current = nextNode(current, yes);
        }
        else{
            bool dummy = false;
            current = nextNode(current, dummy);
        }
    }

    return m_output;
}
