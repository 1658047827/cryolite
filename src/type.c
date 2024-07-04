#include "type.h"
#include <stdlib.h>

QualType voidTy;

VoidType *newVoidType() {
    VoidType *t = (VoidType *)malloc(sizeof(VoidType));
    t->type.kind = TYPE_VOID;
    t->type.canonicalType.t = (Type *)t;
    t->type.canonicalType.quals = 0;
    return t;
}

ArithType *newArithType(ArithKind k) {
    ArithType *t = (ArithType *)malloc(sizeof(ArithType));
    t->type.kind = TYPE_ARITH;
    t->type.canonicalType.t = (Type *)t;
    t->type.canonicalType.quals = 0;
    t->arithKind = k;
    return t;
}