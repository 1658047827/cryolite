#ifndef _CRYOLITE_STMT_H_
#define _CRYOLITE_STMT_H_

#include "expr.h"
#include "decl.h"

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

#endif