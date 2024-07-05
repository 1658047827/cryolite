#include "ast.h"
#include <stdlib.h>

BinaryExpr *newBinaryExpr(BinaryOpKind op, Expr *lhs, Expr *rhs, QualType resTy) {
    BinaryExpr *e = (BinaryExpr *)malloc(sizeof(BinaryExpr));
    e->expr.kind = EXPR_BINARY;
    e->expr.qualType = resTy;
    e->opKind = op;
    e->lhs = lhs;
    e->rhs = rhs;
    return e;
}