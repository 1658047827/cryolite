#ifndef _CRYOLITE_VISITOR_H_
#define _CRYOLITE_VISITOR_H_

#define EXPR(CLASS) class CLASS;
#include "exprNode.def"

#define DECL(CLASS) class CLASS;
#include "declNode.def"

#define STMT(CLASS) class CLASS;
#include "stmtNode.def"

class Visitor {
public:
    virtual void visitUnaryExpr(UnaryExpr *unary) = 0;
    virtual void visitSizeofExpr(SizeofExpr *sizeofExpr) = 0;
    virtual void visitBinaryExpr(BinaryExpr *binary) = 0;
    virtual void visitTernaryExpr(TernaryExpr *ternary) = 0;
    virtual void visitIntegerConstant(IntegerConstant *integer) = 0;
    virtual void visitFloatingConstant(FloatingConstant *floating) = 0;
    virtual void visitCharacterConstant(CharacterConstant *character) = 0;
    virtual void visitStringLiteral(StringLiteral *string) = 0;
    virtual void visitDeclRefExpr(DeclRefExpr *declRef) = 0;

    virtual void visitImplicitCastExpr(ImplicitCastExpr *implicitCast) = 0;

    virtual void visitVarDecl(VarDecl *varDecl) = 0;
    virtual void visitFieldDecl(FieldDecl *fieldDecl) = 0;
    virtual void visitRecordDecl(RecordDecl *recordDecl) = 0;

    virtual void visitBreakStmt(BreakStmt *breakStmt) = 0;
};

class ExprVisitor {
public:
#define EXPR(CLASS) virtual void visit(CLASS *e) = 0;
#include "exprNode.def"
};

class DeclVisitor {
public:
#define DECL(CLASS) virtual void visit(CLASS *d) = 0;
#include "declNode.def"
};

class StmtVisitor {
public:
    virtual void visit(BreakStmt *breakStmt) = 0;
};

#endif