#ifndef _CRYOLITE_VISITOR_H_
#define _CRYOLITE_VISITOR_H_

#define EXPR(CLASS) class CLASS;
#include "ASTNode.def"

#define DECL(CLASS) class CLASS;
#include "ASTNode.def"

#define STMT(CLASS) class CLASS;
#include "ASTNode.def"

class ExprVisitor {
public:
#define EXPR(CLASS) virtual void visit(CLASS *e) = 0;
#include "ASTNode.def"
};

class DeclVisitor {
public:
#define DECL(CLASS) virtual void visit(CLASS *d) = 0;
#include "ASTNode.def"
};

class StmtVisitor {
public:
    virtual void visit(BreakStmt *breakStmt) = 0;
};

#endif