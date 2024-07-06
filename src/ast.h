#ifndef _CRYOLITE_AST_H_
#define _CRYOLITE_AST_H_

#include "type.h"

typedef enum ExprKind {
    EXPR_UNARY,
    EXPR_SIZEOF,
    EXPR_BINARY,
    EXPR_TERNARY,
} ExprKind;

typedef struct Expr {
    ExprKind kind;
    QualType qualType;
} Expr;

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

typedef struct TernaryExpr {
    Expr expr;
    Expr *condExpr;
    Expr *trueExpr;
    Expr *falseExpr;
} TernaryExpr;

typedef struct IntegerConstant {
    Expr expr;
    long long value;
} IntegerConstant;

typedef struct FloatingConstant {
    Expr expr;
    long double value;
} FloatingConstant;

typedef struct CharacterConstant {
    Expr expr;
} CharacterConstant;

typedef struct StringLiteral {
    Expr expr;
} StringLiteral;

typedef enum StmtKind {
    STMT_NULL,
    STMT_DECL,
    STMT_EXPR,
    STMT_BREAK,
    STMT_CONTINUE,
    STMT_COMPOUND,
    STMT_FOR,
    STMT_WHILE,
    STMT_IF,
} StmtKind;

typedef struct Stmt {
    StmtKind kind;
} Stmt;

typedef struct DeclStmt {
    Stmt stmt;
} DeclStmt;

typedef struct ExprStmt {
    Stmt stmt;
    Expr *expr;
} ExprStmt;

typedef struct CompoundStmt {
    Stmt stmt;
} CompoundStmt;

BinaryExpr *newBinaryExpr(BinaryOpKind op, Expr *lhs, Expr *rhs, QualType resTy);

#endif