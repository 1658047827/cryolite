#ifndef _CRYOLITE_AST_H_
#define _CRYOLITE_AST_H_

#include "token.h"
#include "type.h"
#include "visitor.h"
#include <ostream>
#include <vector>

class Decl;
class CompoundStmt;

class Node {
public:
    Node(const SourceLocation &loc) : srcLoc(loc) {}

    virtual void accept(Visitor *v) = 0;

    SourceLocation srcLoc;
};

/**
 * Expressions
 */
class Expr : public Node {
public:
    Expr(const SourceLocation &loc, const QualType &qt) : Node(loc), qtype(qt) {}

    QualType qtype;
};

enum UnaryOpKind {
    PREINC,   // ++i
    PREDEC,   // --i
    POSINC,   // i++
    POSDEC,   // i--
    PLUS,     // +
    MINUS,    // -
    BITNOT,   // ~
    LOGICNOT, // !
    ADDR,     // &
    DEREF,    // *
    // sizeof will be handled in SizeofExpr.
};

class UnaryExpr : public Expr {
public:
    UnaryExpr(const SourceLocation &loc, UnaryOpKind op, Expr *expr);

    void accept(Visitor *v) { v->visitUnaryExpr(this); }

    UnaryOpKind op;
    Expr *operand;

private:
    // TODO: type check
};

// SizeofExpr - The 'sizeof' keyword is quite unique.
// It is semantically closer to a unary expression, but functionally it operates on type trait.
class SizeofExpr : public Expr {
public:
    enum SizeofKind {
        UNARY_EXPR,
        TYPE_NAME
    };

    void accept(Visitor *v) { v->visitSizeofExpr(this); }

    // for "sizeof unary-expression"
    SizeofExpr(const SourceLocation &loc, Expr *expr);
    // for "sizeof ( type-name )"
    SizeofExpr(const SourceLocation &loc, QualType type);

    union SizeofArg {
        Expr *expr;
        QualType type;

        SizeofArg() {}
    } arg;
    SizeofKind sizeofKind;

private:
    // TODO: type check
};

enum BinaryOpKind {
    ADD,      // +
    SUB,      // -
    MUL,      // *
    DIV,      // /
    MOD,      // %
    SHL,      // <<
    SHR,      // >>
    LESS,     // <
    LEQ,      // <=
    GREATER,  // >
    GEQ,      // >=
    EQUAL,    // ==
    NEQ,      // !=
    BITAND,   // &
    BITXOR,   // ^
    BITOR,    // |
    LOGICAND, // &&
    LOGICOR,  // ||
    ASSIGN,   // =
    // Compound assignment will be break into two BinaryExprs.
    COMMA, // ,
};

class BinaryExpr : public Expr {
public:
    BinaryExpr(const SourceLocation &loc, BinaryOpKind op, Expr *lhs, Expr *rhs);

    void accept(Visitor *v) { v->visitBinaryExpr(this); }

    BinaryOpKind op;
    Expr *lhs;
    Expr *rhs;

private:
    // TODO: Finish type checking.
    void checkAdditiveOperator();
    void checkMultiplicativeOperator();
};

// Currently, only the conditional expression is ternary.
class TernaryExpr : public Expr {
public:
    TernaryExpr(const SourceLocation &loc, Expr *condExpr, Expr *trueExpr, Expr *falseExpr);

    void accept(Visitor *v) { v->visitTernaryExpr(this); }

    Expr *condExpr;
    Expr *trueExpr;
    Expr *falseExpr;
};

class IntegerConstant : public Expr {
public:
    IntegerConstant(const SourceLocation &loc, const QualType &qt, unsigned long long val);

    void accept(Visitor *v) { v->visitIntegerConstant(this); }

    unsigned long long value;
};

class FloatingConstant : public Expr {
public:
    FloatingConstant(const SourceLocation &loc, const QualType &qt, long double val);

    void accept(Visitor *v) { v->visitFloatingConstant(this); }

    long double value;
};

class CharacterConstant : public Expr {
public:
    CharacterConstant(const SourceLocation &loc, const QualType &qt, unsigned val);

    void accept(Visitor *v) { v->visitCharacterConstant(this); }

    unsigned value;
};

class StringLiteral : public Expr {
public:
    StringLiteral(const SourceLocation &loc, const QualType &qt, const std::string &str);

    void accept(Visitor *v) { v->visitStringLiteral(this); }

    std::string value;
};

// DeclRefExpr - A reference to a declared variable, function, enum, etc.
// [C99 6.5.1p2] An identifier is a primary expression, provided it has been declared as designating an
// object (in which case it is an lvalue) or a function (in which case it is a function designator).
class DeclRefExpr : public Expr {
public:
    // DeclRefExpr(const SourceLocation &loc, const QualType &qt, Decl *decl);

    // void accept(Visitor *v) { v->visitDeclRefExpr(this); }

    // Decl *decl;
};

class CallExpr : public Expr {
};

class MemberExpr : public Expr {
};

class CastExpr : public Expr {
};

class ImplicitCastExpr : public Expr {
public:
    enum ImplicitKind {
        INTEGRAL_CAST,
        FLOATING_CAST,
        INTEGRAL_TO_FLOATING,
        LVALUE_TO_RVALUE,
        ARRAY_DECAY,    // Array to pointer decay.
        FUNCTION_DECAY, // Function to pointer decay.
    };

    ImplicitCastExpr(const SourceLocation &loc, Expr *from, const QualType &to, ImplicitKind cKind);

    void accept(Visitor *v) { v->visitImplicitCastExpr(this); }

    static ImplicitKind inferArithCastKind(BuiltinType *from, BuiltinType *to);

    Expr *fromExpr;
    ImplicitKind castKind;
};

/**
 * Declarations
 */
class Decl : public Node {
public:
    Decl(const SourceLocation &loc) : Node(loc) {}
};

class EmptyDecl : public Decl {
public:
};

/**
 * FunctionDecl - For function declaration and definition.
 */
class FunctionDecl : public Decl {
public:
    QualType prototype;
    std::vector<void *> paramVarDecls;
    CompoundStmt *stmts;
};

class VarDecl : public Decl {
public:
    QualType type;
    std::string name;
};

class EnumDecl : public Decl {
};

class TypedefDecl : public Decl {
};

class FieldDecl : public Decl {
public:
    FieldDecl(const SourceLocation &loc, QualType qt, const std::string &name = "", Expr *bitWidth = nullptr, unsigned offset = 0);

    void accept(Visitor *v) { v->visitFieldDecl(this); }

    QualType type;
    // bitWidth - Bit-field declaration, declares a member with explicit width, in bits.
    // Adjacent bit-field members may be packed to share and straddle the individual bytes.
    // If struct member is declared normally, bitWidth will be set to nullptr.
    Expr *bitWidth;
    // offset - The offset of a member within a structure.
    unsigned offset;
    std::string fieldName;
};

class RecordDecl : public Decl {
public:
    RecordDecl(const SourceLocation &loc, bool isStruct = true, std::string name = "");

    void accept(Visitor *v) { v->visitRecordDecl(this); }

    // isStruct - True if this record is a struct, false if this record is an union.
    bool isStruct;
    std::string recordName;
    // fields - A vector of FieldDecl/RecordDecl.
    std::vector<Decl *> fields;
};

/**
 * Statements and blocks
 */
class Stmt : public Node {
public:
    Stmt(const SourceLocation &loc) : Node(loc) {}
};

class DeclStmt : public Stmt {
public:
    std::vector<Decl *> decls;
};

class BreakStmt : public Stmt {
public:
    BreakStmt(const SourceLocation &loc) : Stmt(loc) {}

    void accept(Visitor *v) { v->visitBreakStmt(this); }
};

class ContinueStmt : public Stmt {
public:
};

class NullStmt : public Stmt {
public:
};

class ExprStmt : public Stmt {
public:
    Expr *expr;
};

class CompoundStmt : public Stmt {
public:
    std::vector<Stmt *> stmts;
};

class ForStmt : public Stmt {
public:
    CompoundStmt *stmts;
};

/**
 * Translation unit
 */
class TransUnit : public Node {
public:
    std::vector<Decl *> externDecls;
};

class ASTDumper : public Visitor {
public:
    ASTDumper(std::ostream &os) : out(os) {}

    void visitUnaryExpr(UnaryExpr *unary);
    void visitSizeofExpr(SizeofExpr *sizeofExpr);
    void visitBinaryExpr(BinaryExpr *binary);
    void visitTernaryExpr(TernaryExpr *ternary);
    void visitIntegerConstant(IntegerConstant *integer);
    void visitFloatingConstant(FloatingConstant *floating);
    void visitCharacterConstant(CharacterConstant *character);
    void visitStringLiteral(StringLiteral *string);
    void visitDeclRefExpr(DeclRefExpr *declRef);

    void visitImplicitCastExpr(ImplicitCastExpr *implicitCast);

    void visitVarDecl(VarDecl *varDecl);
    void visitFieldDecl(FieldDecl *fieldDecl);
    void visitRecordDecl(RecordDecl *recordDecl);

    void visitBreakStmt(BreakStmt *breakStmt);

    // Helper functions.
    void dumpChild(Node *node);
    void dumpLastChild(Node *node);

    // prefix - Indentation of every line.
    std::string prefix;
    std::ostream &out;
};

#endif
