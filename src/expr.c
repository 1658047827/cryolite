#include "expr.h"

void initExpr(Expr *e, ExprKind kind, QualType t) {
    e->kind = kind;
    e->tr = t;
}

void initIntegerConstant(IntegerConstant *ic, long long value, QualType type) {
    initExpr((Expr *)ic, EXPR_INTEGER, type);
    ic->value = value;
}

void initCharacterConstant(CharacterConstant *cc, unsigned value, _Bool isWide, QualType type) {
    initExpr((Expr *)cc, EXPR_CHARACTER, type);
    cc->value = value;
    cc->isWide = isWide;
}

void initFloatingConstant(FloatingConstant *fc, long double value, _Bool isExact, QualType type) {
    initExpr((Expr *)fc, EXPR_FLOATING, type);
    fc->value = value;
    fc->isExact = isExact;
}

void initUnaryExpr(UnaryExpr *ue, Expr *input, UnaryOpKind op, QualType type) {
    initExpr((Expr *)ue, EXPR_UNARY, type);
    ue->operand = input;
    ue->opKind = op;
}

void initBinaryExpr(BinaryExpr *be, BinaryOpKind op, Expr *lhs, Expr *rhs, QualType ty) {
    initExpr((Expr *)be, EXPR_BINARY, ty);
    be->opKind = op;
    be->lhs = lhs;
    be->rhs = rhs;
}