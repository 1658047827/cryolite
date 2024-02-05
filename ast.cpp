#include "ast.h"

BinaryExpr::BinaryExpr(const SourceLocation &loc, BinaryOpKind op, Expr *lhs, Expr *rhs)
    : Expr(loc), op(op), lhs(lhs), rhs(rhs) {}