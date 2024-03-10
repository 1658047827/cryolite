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

    Expr *implicitCastExprToType(Expr *expr, QualType ty, CastKind cKind);

    // Value transformations
    // lvalueConversion - The type remains the same, but loses const/volatile/restrict-qualifiers.
    Expr *lvalueConversion(Expr *expr);
    Expr *functionArrayConversion(Expr *expr);

    // [C99 6.3.1.1p2] Integer promotions.
    Expr *integerPromotions(Expr *expr);

    // [C99 6.3.1.8] Usual arithmetic conversions.
    QualType usualArithmeticConversions(Expr *&lhs, Expr *&rhs);
    QualType handleFloatingConversions(Expr *&lhs, Expr *&rhs, QualType lhsTy, QualType rhsTy);

    Expr *actOnBinaryOp(Scope *s, SourceLocation opLoc, BinaryOpKind op, Expr *lhs, Expr *rhs);
    Expr *createBuiltinBinaryExpr(Scope *s, SourceLocation opLoc, BinaryOpKind op, Expr *lhs, Expr *rhs);

    QualType checkAdditionOperands(Expr *&lhs, Expr *&rhs, SourceLocation loc);

    Expr *actOnNumericConstant(const Token &tok);

private:
    ASTContext &context;
};

static void diagnoseBitwisePrecedence(BinaryOpKind opK, SourceLocation opLoc, Expr *lhs, Expr *rhs);
static void diagnoseAdditiveInShift(BinaryOpKind opK, SourceLocation opLoc, Expr *lhs, Expr *rhs);

#endif