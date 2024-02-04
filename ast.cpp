#include "ast.h"

BinaryExpr::BinaryExpr(const SourceLocation &loc, BinaryOpKind op, Expr *lhs, Expr *rhs)
    : op(op), lhs(lhs), rhs(rhs) {
    srcLoc = loc;
}