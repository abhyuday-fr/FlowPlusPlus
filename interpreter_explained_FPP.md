# FlowPlusPlus Interpreter.. How It All Works

> A deep dive into the Lexer, Parser, AST, and Graph Walker built for FlowPlusPlus,
> following the spirit of *Crafting Interpreters* by Robert Nystrom.

---

## Table of Contents

1. [The Big Picture](#1-the-big-picture)
2. [Stage 1 — Lexing (Tokenization)](#2-stage-1--lexing-tokenization)
3. [Stage 2 — Parsing](#3-stage-2--parsing)
4. [The AST — What It Is and How We Store It](#4-the-ast--what-it-is-and-how-we-store-it)
5. [Stage 3 — Evaluation](#5-stage-3--evaluation)
6. [Stage 4 — Graph Walking](#6-stage-4--graph-walking)
7. [The Value System](#7-the-value-system)
8. [The Environment (Variable Store)](#8-the-environment-variable-store)
9. [Worked Example — End to End](#9-worked-example--end-to-end)
10. [Differences from Crafting Interpreters](#10-differences-from-crafting-interpreters)

---

## 1. The Big Picture

In a traditional text-based interpreter (like the one in *Crafting Interpreters*),
source code flows through a pipeline:

```
Source Text  →  Lexer  →  Tokens  →  Parser  →  AST  →  Interpreter  →  Output
```

In FlowPlusPlus, the **flowchart itself is the program**. Each node's label
contains a mini-expression. So the pipeline runs **per node**, on demand:

```
Node Label (text)  →  Lexer  →  Tokens  →  Parser  →  Expr (AST)  →  evaluate()  →  Value
```

The graph walker drives everything — it visits nodes one by one, hands each
label to the pipeline above, and uses the result to decide where to go next.

```
                        ┌─────────────────────────────────┐
                        │         FlowScene (canvas)      │
                        │  [Start] → [Process] → [Output] │
                        │               ↓                 │
                        │          [Decision?]            │
                        │          /        \             │
                        │      [Yes path]  [No path]      │
                        └─────────────────────────────────┘
                                        │
                              Interpreter::run()
                                        │
                     ┌──────────────────▼──────────────────┐
                     │  for each node:                      │
                     │    label → Lexer → Parser → evaluate │
                     └──────────────────────────────────────┘
```

---

## 2. Stage 1 — Lexing (Tokenization)

### What is a Lexer?

A **Lexer** (also called a Scanner or Tokenizer) reads raw text character by
character and groups characters into meaningful units called **Tokens**.

For example, the string `x = 5 + 3` becomes:

```
[Identifier:"x"]  [Equal:"="]  [Number:5]  [Plus:"+"]  [Number:3]  [Eof]
```

### Our Token Types

Each token has a **type** (what kind of thing it is) and a **lexeme** (the raw
text it came from). Number and String tokens also carry a **literal value**
already converted to a `Value`.

```cpp
struct Token {
    enum class Type {
        Number, String, True, False, Nil,   // Literals
        Identifier,                          // Variable names
        Plus, Minus, Star, Slash, Percent,   // Arithmetic
        EqualEqual, BangEqual,               // Equality
        Less, LessEqual, Greater, GreaterEqual, // Comparison
        Bang, And, Or,                       // Logic
        Equal,                               // Assignment  =
        LeftParen, RightParen,               // Grouping
        Print, Input,                        // Keywords
        Eof                                  // End of input
    };

    Type    type;
    QString lexeme;   // raw text, e.g. "5", "+", "hello"
    Value   literal;  // pre-parsed value for Number/String tokens
};
```

### How the Lexer Works — Step by Step

The Lexer holds:
- `m_source` — the full input string
- `m_start` — where the current token started
- `m_current` — where the scanner is right now

It loops calling `advance()` which returns the next character and moves
`m_current` forward. Based on what it sees, it decides what token to emit.

```
Source:  "x = 42 + y"
          ^
          m_current starts here

Step 1:  advance() → 'x'
         x is a letter → call identifier()
         identifier() keeps advancing while peek() is alphanumeric
         Result: Token{ Identifier, "x" }

Step 2:  advance() → ' '  (space)
         isSpace() → skip, continue

Step 3:  advance() → '='
         match('=') checks if next char is also '='
         It is NOT → emit Token{ Equal, "=" }

Step 4:  advance() → ' '  → skip

Step 5:  advance() → '4'
         isDigit() → call number()
         number() keeps advancing while peek() is a digit
         Also handles '.' for decimals
         "42" → toDouble() → 42.0
         Result: Token{ Number, "42", Value(42.0) }

... and so on
```

#### Special cases in the Lexer

**Two-character operators** — things like `==`, `!=`, `<=`, `>=` are handled
with `match()`:

```cpp
case '=':
    tokens << makeToken(match('=')
                ? Token::Type::EqualEqual   // saw ==
                : Token::Type::Equal);      // just =
    break;
```

`match()` peeks at the next character. If it matches, it consumes it and
returns true. Otherwise it returns false and leaves the cursor alone.

**String literals** — the Lexer sees `"`, then keeps advancing until it finds
the closing `"`. It strips the quotes and stores the inner text:

```cpp
Token Lexer::string() {
    while (!isAtEnd() && peek() != '"') advance();
    advance(); // closing "
    // m_start+1 skips the opening quote, -2 removes both quotes
    QString val = m_source.mid(m_start + 1, m_current - m_start - 2);
    return makeToken(Token::Type::String, Value(val));
}
```

**Keywords** — after reading an identifier, we check if the word is a
reserved keyword:

```cpp
if (text == "true")  return makeToken(Token::Type::True,  Value(true));
if (text == "false") return makeToken(Token::Type::False, Value(false));
if (text == "print") return makeToken(Token::Type::Print);
if (text == "input") return makeToken(Token::Type::Input);
// Otherwise it's a variable name (Identifier)
```

---

## 3. Stage 2 — Parsing

### What is a Parser?

The **Parser** takes the flat list of tokens from the Lexer and builds a
**tree** that represents the structure and precedence of the expression.

For example, `5 + 3 * 2` should be evaluated as `5 + (3 * 2)`, not `(5 + 3) * 2`.
The parser enforces this using a technique called **Recursive Descent**.

### Recursive Descent Parsing

The core idea: each level of **operator precedence** gets its own function.
Lower precedence operators (like `or`, `and`) call higher precedence functions
(like `+`, `*`) to parse their operands. This naturally builds the right tree.

Our precedence table, from lowest to highest:

```
Level   │ Operations          │ Function
────────┼─────────────────────┼──────────────
1 (low) │ = (assignment)      │ assignment()
2       │ or                  │ logicalOr()
3       │ and                 │ logicalAnd()
4       │ == !=               │ equality()
5       │ < <= > >=           │ comparison()
6       │ + -                 │ term()
7       │ * / %               │ factor()
8       │ ! - (unary)         │ unary()
9 (high)│ literals, variables │ primary()
```

### How Recursive Descent Works

Take `x = 5 + 3 > 2`:

```
expression()
  └─ assignment()
       sees Identifier "x" then "="
       └─ parses right side: logicalOr()
            └─ logicalAnd()
                 └─ equality()
                      └─ comparison()
                           └─ left = term()
                                └─ left = factor() → unary() → primary() → 5
                                   sees "+" → right = factor() → 3
                                   returns Binary(+, 5, 3)
                           sees ">" → right = term() → 2
                           returns Binary(>, Binary(+,5,3), 2)
       returns Assign("x", Binary(>, Binary(+,5,3), 2))
```

### The Parser's Core Loop Pattern

Every parsing function follows this pattern:

```cpp
QSharedPointer<Expr> Parser::term() {
    auto expr = factor();             // parse higher-precedence left side

    while (match({Plus, Minus})) {    // while we see a + or -
        QString op = previous().lexeme;
        auto right = factor();        // parse right side at same level
        // Build a Binary node
        auto e = QSharedPointer<Expr>::create();
        e->kind  = Expr::Kind::Binary;
        e->op    = op;
        e->left  = expr;
        e->right = right;
        expr = e;                     // the new node becomes the left side
    }

    return expr;
}
```

This `while` loop handles left-associativity: `1 + 2 + 3` becomes
`Binary(+, Binary(+, 1, 2), 3)` — left to right.

### Special Parse Cases

**Assignment** — `x = value`:
We first parse the left side as an expression. If we then see `=`, we check
that the left side was a Variable (you can only assign to a name, not `5 = x`).
Then we parse the right side and build an Assign node.

**Print** — `print expr`:
After seeing the `print` keyword token, we just parse the rest as an
expression and wrap it in a Print node.

**Input** — `input varname`:
After seeing `input`, we expect an Identifier (the variable name to store into).

---

## 4. The AST — What It Is and How We Store It

### What is an AST?

An **Abstract Syntax Tree** (AST) is a tree where:
- Each **node** represents one operation or value
- **Children** are the operands of that operation
- The **root** represents the whole expression

For `x = 5 + 3`:

```
        Assign
        /    \
       x    Binary(+)
            /     \
        Literal   Literal
           5         3
```

### How We Store It — The `Expr` Struct

In *Crafting Interpreters*, Nystrom uses a separate class per node type with
the Visitor pattern. We simplify this into **one struct with a `Kind` enum**:

```cpp
struct Expr {
    enum class Kind {
        Literal,    // a raw value: 42, "hello", true
        Variable,   // a variable name: x
        Assign,     // x = expr
        Unary,      // -x or !x
        Binary,     // x + y, x > y, x == y
        Logical,    // x and y, x or y
        Print,      // print expr
        Input,      // input varname
    };

    Kind    kind;
    Value   value;   // used by Literal
    QString name;    // used by Variable, Assign, Input
    QString op;      // used by Binary, Unary, Logical: "+", ">", "and", etc.
    QSharedPointer<Expr> left;   // left child
    QSharedPointer<Expr> right;  // right child (also used by Unary, Print)
};
```

`QSharedPointer<Expr>` is used instead of raw pointers so memory is managed
automatically — no manual `delete` needed.

### Example ASTs

**`42`**
```
Literal(42)
```

**`x + y`**
```
Binary("+")
├── Variable("x")
└── Variable("y")
```

**`print x > 5`**
```
Print
└── Binary(">")
    ├── Variable("x")
    └── Literal(5)
```

**`a = b + c * 2`**
```
Assign("a")
└── Binary("+")
    ├── Variable("b")
    └── Binary("*")
        ├── Variable("c")
        └── Literal(2)
```

Notice how `*` binds tighter than `+` — the tree structure encodes precedence
without any runtime checks needed.

---

## 5. Stage 3 — Evaluation

### The `evaluate()` Function

Once we have the AST, `evaluate()` walks it recursively and computes a `Value`.
It's a simple `switch` on the node's `Kind`:

```cpp
Value Interpreter::evaluate(const QSharedPointer<Expr> &expr) {
    switch (expr->kind) {

    case Expr::Kind::Literal:
        return expr->value;         // already a Value, just return it

    case Expr::Kind::Variable:
        return m_env->get(expr->name);  // look up in environment

    case Expr::Kind::Assign: {
        Value val = evaluate(expr->right);   // evaluate RHS first
        m_env->define(expr->name, val);      // store in environment
        return val;
    }

    case Expr::Kind::Binary:
        return evalBinary(expr->op,
                          evaluate(expr->left),   // evaluate both sides
                          evaluate(expr->right));  // then apply operator

    case Expr::Kind::Unary: {
        Value right = evaluate(expr->right);
        if (expr->op == "-") return Value(-right.asNumber());
        if (expr->op == "!") return Value(!right.isTruthy());
        return Value();
    }

    case Expr::Kind::Logical: {
        Value left = evaluate(expr->left);
        // Short-circuit: for "or", if left is truthy, skip right
        if (expr->op == "or")
            return left.isTruthy() ? left : evaluate(expr->right);
        // For "and", if left is falsy, skip right
        return !left.isTruthy() ? left : evaluate(expr->right);
    }

    case Expr::Kind::Print: {
        Value val = evaluate(expr->right);
        m_output << val.toString();   // add to output lines
        return val;
    }

    case Expr::Kind::Input: {
        // Pop up a dialog, read user input, store in variable
        QString userInput = requestInput(expr->name);
        Value val = /* parse as number if possible, else string */;
        m_env->define(expr->name, val);
        return val;
    }
    }
}
```

The key insight: **tree structure drives evaluation order**. For `Binary(+, left, right)`,
we evaluate `left` and `right` first (recursion), then apply `+`. This
automatically handles precedence because the tree was already built correctly.

### Binary Operator Evaluation

```cpp
Value Interpreter::evalBinary(const QString &op, const Value &l, const Value &r) {
    if (op == "+") {
        if (l.isNumber() && r.isNumber())
            return Value(l.asNumber() + r.asNumber());
        return Value(l.toString() + r.toString());  // string concatenation
    }
    if (op == "-" && l.isNumber() && r.isNumber())
        return Value(l.asNumber() - r.asNumber());
    if (op == "==") return Value(l == r);
    if (op == ">" && l.isNumber() && r.isNumber())
        return Value(l.asNumber() > r.asNumber());
    // ... etc
}
```

`+` does double duty: numbers add, strings concatenate. This mirrors how
Python and JavaScript work.

---

## 6. Stage 4 — Graph Walking

### Finding the Start Node

The interpreter scans all items in the `FlowScene` looking for a
`StartStopNode` in `Start` mode:

```cpp
FlowNode* Interpreter::findStartNode() {
    for (QGraphicsItem *item : m_scene->items()) {
        StartStopNode *n = dynamic_cast<StartStopNode*>(item);
        if (n && n->mode() == StartStopNode::Mode::Start)
            return n;
    }
    return nullptr;
}
```

### The Main Loop

```cpp
QStringList Interpreter::run() {
    FlowNode *current = findStartNode();

    int steps = 0;
    while (current && steps++ < 1000) {

        // Stop node? We're done.
        StartStopNode *ss = dynamic_cast<StartStopNode*>(current);
        if (ss && ss->mode() == StartStopNode::Mode::Stop) break;

        // Execute this node's label expression
        Value result = executeNode(current);

        // For Decision nodes, result determines Yes or No branch
        DecisionNode *dn = dynamic_cast<DecisionNode*>(current);
        if (dn) {
            bool takeYes = result.isTruthy();
            current = nextNode(current, takeYes);
        } else {
            bool dummy = false;
            current = nextNode(current, dummy);
        }
    }
}
```

The step limit of 1000 prevents infinite loops — if a flowchart loops forever,
we stop and report an error.

### Following Connections

Each `FlowNode` maintains a list of `FlowConnection` objects. To find the next
node, we scan the connections looking for ones where `fromNode() == current`:

```cpp
FlowNode* Interpreter::nextNode(FlowNode *current, bool takeYes) {
    DecisionNode *dn = dynamic_cast<DecisionNode*>(current);

    for (FlowConnection *conn : current->connections()) {
        if (conn->fromNode() != current) continue;

        if (dn) {
            // isYesConnection() was stored at connection creation time
            if (takeYes && conn->isYesConnection())  return conn->toNode();
            if (!takeYes && !conn->isYesConnection()) return conn->toNode();
        } else {
            return conn->toNode();  // non-decision: just take the only connection
        }
    }
    return nullptr;
}
```

### Executing a Node

Each node type has different semantics:

```cpp
Value Interpreter::executeNode(FlowNode *node) {
    QString label = node->label().trimmed();
    if (label.isEmpty()) return Value();

    IONode *io = dynamic_cast<IONode*>(node);
    if (io) {
        if (io->mode() == IONode::Mode::Output)
            label = "print " + label;   // auto-prefix "print"
        else
            label = "input " + label;   // auto-prefix "input"
    }
    // Process and Decision nodes: label is evaluated as-is
    // Decision result is used to pick Yes/No branch

    Lexer  lexer(label);
    Parser parser(lexer.tokenize());
    auto   expr = parser.parse();
    return evaluate(expr);
}
```

| Node Type  | Label example     | What happens                              |
|------------|-------------------|-------------------------------------------|
| Start      | "Start"           | Skipped (no expression executed)          |
| Stop       | "Stop"            | Terminates the walk                       |
| Process    | `x = 5 + 3`       | Evaluates assignment, stores x=8          |
| Decision   | `x > 5`           | Evaluates condition → truthy = Yes branch |
| Output     | `"Hello"` or `x`  | Auto-prefixed with `print`, prints value  |
| Input      | `x`               | Auto-prefixed with `input`, asks user     |

---

## 7. The Value System

A `Value` is what the interpreter computes. It uses `std::variant` to hold
one of four possible types:

```cpp
std::variant<Nil, Number, Bool, Str> data;
//           │      │      │     │
//           │      │      │     └─ QString
//           │      │      └─────── bool
//           │      └────────────── double
//           └───────────────────── std::monostate (nothing)
```

This is a **tagged union** — only one type is active at a time, and you must
check which one before using it.

### Truthiness

Mirroring *Crafting Interpreters* exactly:

```cpp
bool isTruthy() const {
    if (isNil())  return false;   // nil is falsy
    if (isBool()) return asBool(); // false is falsy, true is truthy
    return true;                   // everything else is truthy
                                   // including 0, empty string
}
```

This means `0` is **truthy** in FlowPlusPlus (unlike C/Python). Only `nil`
and `false` are falsy.

---

## 8. The Environment (Variable Store)

The `Environment` stores variables as a `QHash<QString, Value>` — a simple
name → value map. It also supports **nested scopes** via a parent pointer:

```cpp
class Environment {
    QHash<QString, Value>       m_values;
    QSharedPointer<Environment> m_parent;  // enclosing scope
};
```

### Scope Chain

When you look up a variable, it first checks the current scope. If not found,
it walks up to the parent:

```
get("x"):
  current scope: { y: 5 }        → not found
  parent scope:  { x: 10, z: 3 } → found! return 10
```

This is exactly how closures and nested functions work in real languages.
Currently FlowPlusPlus uses a single flat scope (no nested scopes yet), but
the structure is ready for it.

### define vs assign

```cpp
// define: always creates in current scope (even if name exists in parent)
env->define("x", Value(5));

// assign: walks up the chain to find where x is, updates it there
env->assign("x", Value(10));

// get: walks up the chain to find and return x
Value v = env->get("x");
```

---

## 9. Worked Example — End to End

Let's trace through this flowchart:

```
[Start]
   ↓
[Process]   label: x = 3
   ↓
[Decision]  label: x > 2
  / \
Yes   No
 ↓     ↓
[Output]  [Output]
"big"     "small"
   \     /
    [Stop]
```

### Step 1: Find Start node → enter loop

### Step 2: Execute Process node `x = 3`

```
Lexer("x = 3")
→ [Identifier:"x"] [Equal:"="] [Number:3.0] [Eof]

Parser:
  assignment()
    logicalOr() → ... → primary() → Variable("x")  ← left side
    sees Equal "="
    right = logicalOr() → ... → Literal(3.0)
    returns Assign{ name:"x", right:Literal(3.0) }

evaluate(Assign):
  val = evaluate(Literal(3.0)) = Value(3.0)
  env->define("x", Value(3.0))
  return Value(3.0)
```

Environment is now: `{ x: 3.0 }`

### Step 3: Execute Decision node `x > 2`

```
Lexer("x > 2")
→ [Identifier:"x"] [Greater:">"] [Number:2.0] [Eof]

Parser:
  comparison()
    left = term() → ... → Variable("x")
    sees Greater ">"
    right = term() → ... → Literal(2.0)
    returns Binary{ op:">", left:Variable("x"), right:Literal(2.0) }

evaluate(Binary):
  l = evaluate(Variable("x")) = env->get("x") = Value(3.0)
  r = evaluate(Literal(2.0))  = Value(2.0)
  evalBinary(">", 3.0, 2.0) = Value(true)

result.isTruthy() = true → take Yes branch
```

### Step 4: Execute Output node `"big"`

```
label becomes: "print \"big\""

Lexer → [Print] [String:"big"] [Eof]
Parser → Print{ right: Literal("big") }

evaluate(Print):
  val = evaluate(Literal("big")) = Value("big")
  m_output << "big"
```

### Step 5: Reach Stop node → break

### Output dialog shows:
```
big
```

---

## 10. Differences from Crafting Interpreters

| Aspect | Crafting Interpreters (Lox) | FlowPlusPlus |
|--------|----------------------------|--------------|
| Input | Text file / REPL | Node labels on a canvas |
| AST nodes | Separate class per node type | Single `Expr` struct with `Kind` enum |
| Visitor pattern | Yes (Expr::Visitor) | No — simple `switch` in `evaluate()` |
| Statements | Separate from expressions | Everything is an expression |
| Control flow | `if`, `while`, `for` keywords | Visual arrows + Decision nodes |
| Variables | Explicit `var` declaration | Auto-declared on first assignment |
| Functions | Yes (Chapter 10+) | Not yet implemented |
| Closures | Yes | Not yet (Environment ready) |
| Error handling | Lox error reporting | Error string added to output |

The biggest architectural difference is that **the flowchart IS the control flow**.
There are no `if` or `while` keywords — the graph structure itself encodes
branching (Decision nodes) and looping (connections that go backward).

---

*FlowPlusPlus : making programming visual, one node at a time.*