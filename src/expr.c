#include "expr.h"
#include <stdlib.h>

void initExpr(Expr *obj, ExprKind kind, QualType t) {
    obj->kind = kind;
    obj->tr = t;
}

void initIntegerConstant(IntegerConstant *obj, long long value, QualType type) {
    initExpr((Expr *)obj, EXPR_INTEGER, type);
    obj->value = value;
}

void initCharacterConstant(CharacterConstant *obj, unsigned value, _Bool isWide, QualType type) {
    initExpr((Expr *)obj, EXPR_CHARACTER, type);
    obj->value = value;
    obj->isWide = isWide;
}

void initBinaryExpr(BinaryExpr *obj, BinaryOpKind op, Expr *lhs, Expr *rhs, QualType ty) {
    initExpr((Expr *)obj, EXPR_BINARY, ty);
    obj->opKind = op;
    obj->lhs = lhs;
    obj->rhs = rhs;
}