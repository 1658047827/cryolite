#ifndef _CRYOLITE_SEMANTIC_H_
#define _CRYOLITE_SEMANTIC_H_

#include "ast.h"
#include "scope.h"
#include "token.h"

class Expr;

class Semantic {
public:
    Semantic(ASTContext &ctxt) : context(ctxt) {}

    void actOnPopScope(SourceLocation loc, Scope *s);

    void defaultFunctionArrayConversion(Expr *&expr);
    Expr *usualUnaryConversions(Expr *&expr);
    QualType usualArithmeticConversions(Expr *lhs, Expr *rhs);

    Expr *actOnBinaryOp(Scope *s, SourceLocation loc, BinaryOpKind op, Expr *lhs, Expr *rhs);
    Expr *buildBinaryExpr(Scope *s, SourceLocation loc, BinaryOpKind op, Expr *lhs, Expr *rhs);

    QualType checkAdditionOperands(Expr *lhs, Expr *rhs, SourceLocation loc);

private:
    ASTContext &context;
};

static void diagnoseBitwisePrecedence(BinaryOpKind opK, SourceLocation opLoc, Expr *lhs, Expr *rhs);
static void diagnoseAdditiveInShift(BinaryOpKind opK, SourceLocation opLoc, Expr *lhs, Expr *rhs);

#endif