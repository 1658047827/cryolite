#include "ast.h"

BinaryExpr::BinaryExpr(const SourceLocation &loc, BinaryOpKind op, Expr *lhs, Expr *rhs)
    : Expr(loc), op(op), lhs(lhs), rhs(rhs) {}

TernaryExpr::TernaryExpr(const SourceLocation &loc, Expr *condExpr, Expr *trueExpr, Expr *falseExpr)
    : Expr(loc), condExpr(condExpr), trueExpr(trueExpr), falseExpr(falseExpr) {}
