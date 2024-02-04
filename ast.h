#ifndef _CRYOLITE_AST_H_
#define _CRYOLITE_AST_H_

#include "token.h"
#include "visitor.h"
#include <vector>

class Node {
public:
    virtual void accept(Visitor *v) = 0;

    SourceLocation srcLoc;
};

/**
 * Expression
 */
class Expression : public Node {
public:
};

class BinaryOperator : public Node {
};

class UnaryOperator : public Node {
};

/**
 * Statement
 */
class Statement : public Node {
public:
};

class NullStmt : public Statement {
};

class ExpressionStatement : public Statement {
public:
    Expression *expr;
};

class CompoundStmt : public Statement {
};




class FuncDef : public Node {
};

class ExternDecl : public Node {
};

class TransUnit : public Node {
public:
};

#endif
