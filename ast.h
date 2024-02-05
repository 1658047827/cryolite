#ifndef _CRYOLITE_AST_H_
#define _CRYOLITE_AST_H_

#include "token.h"
#include "visitor.h"
#include <vector>

class Node {
public:
    Node(const SourceLocation &loc) : srcLoc(loc) {}

    virtual void accept(Visitor *v) = 0;

    SourceLocation srcLoc;
};

/**
 * Expressions
 */
class Expr : public Node {
public:
    Expr(const SourceLocation &loc) : Node(loc) {}
};

class UnaryExpr : public Expr {
};

enum BinaryOpKind {
    ADD,   // +
    SUB,   // -
    MUL,   // *
    DIV,   // /
    MOD,   // %
    SHL,   // <<
    SHR,   // >>
    COMMA, // ,
};

class BinaryExpr : public Expr {
public:
    BinaryExpr(const SourceLocation &loc, BinaryOpKind op, Expr *lhs, Expr *rhs);

    void accept(Visitor *v){};

    BinaryOpKind op;
    Expr *lhs;
    Expr *rhs;

private:
    // TODO type check
};

// Only the conditional expression is ternary currently.
class TernaryExpr : public Expr {
public:
    Expr *condition;
    Expr *trueExpr;
    Expr *falseExpr;
};

/**
 * Declarations
 */

/**
 * Statements and blocks
 */
class Stmt : public Node {
};

class NullStmt : public Stmt {
};

class ExprStmt : public Stmt {
public:
    Expr *expr;
};

class CompoundStmt : public Stmt {
public:
    std::vector<Stmt *> stmts;
};

/**
 * External definitions
 */
class FuncDef : public Node {
};

class ExternDecl : public Node {
};

class TransUnit : public Node {
public:
    std::vector<ExternDecl *> externDecls;
};

#endif
