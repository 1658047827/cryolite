#ifndef _CRYOLITE_EXPR_H_
#define _CRYOLITE_EXPR_H_

#include "type.h"

typedef enum ExprKind {
    EXPR_DECLREF,
    EXPR_INTEGER,
    EXPR_CHARACTER,
    EXPR_FLOATING,
    EXPR_STRING,
    EXPR_UNARY,
    EXPR_SIZEOF,
    EXPR_BINARY,
    EXPR_TERNARY,
} ExprKind;

typedef struct Expr {
    ExprKind kind;
    QualType tr;
} Expr;

void initExpr(Expr *obj, ExprKind kind, QualType t);

// DeclRefExpr - A reference to a declared variable, function, enum, etc.
// [C99 6.5.1p2] An identifier is a primary expression, provided it has been declared as designating an
// object (in which case it is an lvalue) or a function (in which case it is a function designator).
typedef struct DeclRefExpr {
    Expr expr;
} DeclRefExpr;

typedef struct IntegerConstant {
    Expr expr;
    long long value;
} IntegerConstant;

void initIntegerConstant(IntegerConstant *obj, long long value, QualType type);

typedef struct CharacterConstant {
    Expr expr;
    unsigned value;
    _Bool isWide;
} CharacterConstant;

void initCharacterConstant(CharacterConstant *obj, unsigned value, _Bool isWide, QualType type);

typedef struct FloatingConstant {
    Expr expr;
    long double value;
    _Bool IsExact
} FloatingConstant;

typedef struct StringLiteral {
    Expr expr;
    const char *strData;
    unsigned byteLength;
} StringLiteral;

void initStringLiteral();

typedef enum UnaryOpKind {
    UNARY_POSINC,
    UNARY_POSDEC,
    UNARY_PREINC,
    UNARY_PREDEC,
    UNARY_PLUS,
    UNARY_MINUS,
    UNARY_BITNOT,
    UNARY_LOGICNOT,
    UNARY_ADDR,
    UNARY_DEREF
} UnaryOpKind;

typedef struct UnaryExpr {
    Expr expr;
    UnaryOpKind opKind;
} UnaryExpr;

typedef enum SizeofKind {
    SIZEOF_EXPR,
    SIZEOF_TYPE
} SizeofKind;

// SizeofExpr - The 'sizeof' keyword is quite unique.
// It is semantically closer to a unary expression, but functionally it operates on type trait.
typedef struct SizeofExpr {
    Expr expr;
    SizeofKind sizeofKind;
} SizeofExpr;

typedef enum BinaryOpKind {
    BINARY_ADD,
    BINARY_SUB,
    BINARY_MUL,
    BINARY_DIV,
    BINARY_MOD,
    BINARY_SHL,
    BINARY_SHR,
    BINARY_LESS,
    BINARY_LEQ,
    BINARY_GREATER,
    BINARY_GEQ,
    BINARY_EQUAL,
    BINARY_NEQ,
    BINARY_BITAND,
    BINARY_BITXOR,
    BINARY_BITOR,
    BINARY_LOGICAND,
    BINARY_LOGICOR,
    BINARY_ASSIGN,
    BINARY_COMMA,
} BinaryOpKind;

typedef struct BinaryExpr {
    Expr expr;
    BinaryOpKind opKind;
    Expr *lhs;
    Expr *rhs;
} BinaryExpr;

void initBinaryExpr(BinaryExpr *obj, BinaryOpKind op, Expr *lhs, Expr *rhs, QualType ty);

typedef struct TernaryExpr {
    Expr expr;
    Expr *condExpr;
    Expr *trueExpr;
    Expr *falseExpr;
} TernaryExpr;

// ArraySubscriptExpr - [C99 6.5.2.1] Array Subscripting.
typedef struct ArraySubscriptExpr {
    Expr expr;
} ArraySubscriptExpr;

typedef struct CallExpr {
    Expr expr;
} CallExpr;

typedef struct MemberExpr {
    Expr expr;
} MemberExpr;

#endif