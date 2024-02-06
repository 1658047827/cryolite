#ifndef _CRYOLITE_AST_H_
#define _CRYOLITE_AST_H_

#include "token.h"
#include "type.h"
#include "visitor.h"
#include <ostream>
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

    QualType qt;
};

enum UnaryOpKind {
    PREINC, // ++i
    PREDEC, // --i
    POSINC, // i++
    POSDEC, // i--
};

class UnaryExpr : public Expr {
public:
    UnaryExpr(const SourceLocation &loc, UnaryOpKind op, Expr *expr);

    void accept(Visitor *v) { v->visitUnaryExpr(this); }

    UnaryOpKind op;
    Expr *operand;
};

enum BinaryOpKind {
    ADD,      // +
    SUB,      // -
    MUL,      // *
    DIV,      // /
    MOD,      // %
    SHL,      // <<
    SHR,      // >>
    LESS,     // <
    LEQ,      // <=
    GREATER,  // >
    GEQ,      // >=
    EQUAL,    // ==
    NEQ,      // !=
    BITAND,   // &
    BITXOR,   // ^
    BITOR,    // |
    LOGICAND, // &&
    LOGICOR,  // ||
    ASSIGN,   // =
    // Compound assignment will be break into two BinaryExprs.
    COMMA, // ,
};

class BinaryExpr : public Expr {
public:
    BinaryExpr(const SourceLocation &loc, BinaryOpKind op, Expr *lhs, Expr *rhs);

    void accept(Visitor *v) { v->visitBinaryExpr(this); };

    BinaryOpKind op;
    Expr *lhs;
    Expr *rhs;

private:
    // TODO type check
};

// Currently, only the conditional expression is ternary.
class TernaryExpr : public Expr {
public:
    TernaryExpr(const SourceLocation &loc, Expr *condExpr, Expr *trueExpr, Expr *falseExpr);

    void accept(Visitor *v) { v->visitTernaryExpr(this); };

    Expr *condExpr;
    Expr *trueExpr;
    Expr *falseExpr;
};

class IntegerConstant : public Expr {
public:
    IntegerConstant(const SourceLocation &loc, unsigned long long val);

    void accept(Visitor *v) { v->visitIntegerConstant(this); };

    unsigned long long value;
};

class FloatingConstant : public Expr {
public:
    FloatingConstant(const SourceLocation &loc, long double val);

    void accept(Visitor *v) { v->visitFloatingConstant(this); };

    long double value;
};

class StringLiteral : public Expr {
public:
    StringLiteral(const SourceLocation &loc, const std::string &str);

    void accept(Visitor *v){};

    std::string value;
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

class ASTDump : public Visitor {
public:
    ASTDump(std::ostream &os) : out(os) {}

    void visitUnaryExpr(UnaryExpr *unary);
    void visitBinaryExpr(BinaryExpr *binary);
    void visitTernaryExpr(TernaryExpr *ternary);
    void visitIntegerConstant(IntegerConstant *integer);
    void visitFloatingConstant(FloatingConstant *floating);
    void visitStringLiteral(StringLiteral* string);

    std::string prefix;
    std::ostream &out;
};

#endif
