#ifndef _CRYOLITE_AST_H_
#define _CRYOLITE_AST_H_

#include "Token.h"
#include "Type.h"
#include "Visitor.h"
#include <ostream>
#include <vector>

class Decl;
class CompoundStmt;

class ASTContext;

class Node {
public:
    Node(SourceLocation loc) : srcLoc(loc) {}
    SourceLocation getSrcLoc() const { return srcLoc; }

private:
    SourceLocation srcLoc;
};

/**
 * Expressions
 */
class Expr : public Node {
public:
    Expr(SourceLocation loc, QualType qt) : Node(loc), qtype(qt) {}

    QualType getQualType() const { return qtype; }

    virtual void accept(ExprVisitor *v) = 0;

    // TODO: More evaluators, more checking, ref Clang.
    bool evaluateAsUnsignedInt(std::size_t &evalRet, ASTContext &ctx) const;
    bool evaluateAsFloating(long double &evalRet, ASTContext &ctx) const;

private:
    QualType qtype;
};

template <typename Derived>
class VisitableExpr : public Expr {
public:
    template <typename... Args>
    VisitableExpr(Args &&...args) : Expr(std::forward<Args>(args)...) {}

    void accept(ExprVisitor *v) { v->visit(static_cast<Derived *>(this)); }
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

class UnaryExpr : public VisitableExpr<UnaryExpr> {
public:
    UnaryExpr(SourceLocation loc, UnaryOpKind op, Expr *expr);

    UnaryOpKind getOp() const { return op; }
    Expr *getOperand() const { return operand; }

private:
    UnaryOpKind op;
    Expr *operand;
};

// SizeofExpr - The 'sizeof' keyword is quite unique.
// It is semantically closer to a unary expression, but functionally it operates on type trait.
class SizeofExpr : public VisitableExpr<SizeofExpr> {
public:
    enum SizeofKind {
        UNARY_EXPR,
        TYPE_NAME
    };

    // for "sizeof unary-expression"
    SizeofExpr(SourceLocation loc, QualType resultTy, Expr *expr);
    // for "sizeof ( type-name )"
    SizeofExpr(SourceLocation loc, QualType resultTy, QualType type);

private:
    union SizeofArg {
        Expr *expr;
        QualType type;

        SizeofArg() {}
    } arg;
    SizeofKind sizeofKind;
};

enum BinaryOpKind {
#define BINARY(NAME, REPR) NAME,
#include "OperatorKind.def"
};

class BinaryExpr : public VisitableExpr<BinaryExpr> {
public:
    BinaryExpr(SourceLocation loc, BinaryOpKind op, Expr *lhs, Expr *rhs);

    bool isAdditiveOp() const { return isAdditiveOp(op); }
    static bool isAdditiveOp(BinaryOpKind opK) { return opK == ADD || opK == SUB; }

    bool isComparisonOp() const { return isComparisonOp(op); }
    static bool isComparisonOp(BinaryOpKind opK) { return opK >= LESS && opK <= NEQ; }

    bool isBitwiseOp() const { return isBitwiseOp(op); }
    static bool isBitwiseOp(BinaryOpKind opK) { return opK >= BITAND && opK <= BITOR; }

    std::string_view getOpStr() const { return getOpStr(op); }
    static std::string_view getOpStr(BinaryOpKind opK);

    BinaryOpKind op;
    Expr *lhs;
    Expr *rhs;
};

// Currently, only the conditional expression is ternary.
class TernaryExpr : public VisitableExpr<TernaryExpr> {
public:
    TernaryExpr(SourceLocation loc, Expr *condExpr, Expr *trueExpr, Expr *falseExpr);

    Expr *condExpr;
    Expr *trueExpr;
    Expr *falseExpr;
};

class IntegerConstant : public VisitableExpr<IntegerConstant> {
public:
    IntegerConstant(SourceLocation loc, QualType qt, unsigned long long val);

    unsigned long long getValue() const { return value; }

private:
    unsigned long long value;
};

class FloatingConstant : public VisitableExpr<FloatingConstant> {
public:
    FloatingConstant(SourceLocation loc, QualType qt, long double val);

    long double getValue() const { return value; }

private:
    long double value;
};

class CharacterConstant : public VisitableExpr<CharacterConstant> {
public:
    CharacterConstant(SourceLocation loc, QualType qt, int val);

    int value;
};

class StringLiteral : public VisitableExpr<StringLiteral> {
public:
    StringLiteral(SourceLocation loc, QualType qt, const std::string &str);

    std::string content;
};

// DeclRefExpr - A reference to a declared variable, function, enum, etc.
// [C99 6.5.1p2] An identifier is a primary expression, provided it has been declared as designating an
// object (in which case it is an lvalue) or a function (in which case it is a function designator).
class DeclRefExpr : public VisitableExpr<DeclRefExpr> {
public:
    // DeclRefExpr(const SourceLocation &loc, const QualType &qt, Decl *decl);

    // void accept(Visitor *v) { v->visitDeclRefExpr(this); }

    // Decl *decl;
};

class ParenExpr : public VisitableExpr<ParenExpr> {
public:
    ParenExpr(SourceLocation loc, Expr *subExpr);

private:
    Expr *subExpr;
};

class CallExpr : public VisitableExpr<CallExpr> {
};

class MemberExpr : public VisitableExpr<MemberExpr> {
};

enum CastKind {
    INTEGRAL_CAST,
    FLOATING_CAST,
    INTEGRAL_TO_FLOATING,
    LVALUE_TO_RVALUE,
    ARRAY_TO_POINTER_DECAY,
    FUNCTION_TO_POINTER_DECAY,
};

class CastExpr : public VisitableExpr<CastExpr> {
};

class ImplicitCastExpr : public VisitableExpr<ImplicitCastExpr> {
public:
    ImplicitCastExpr(Expr *from, QualType to, CastKind cKind);

    Expr *getFromExpr() const { return fromExpr; }
    CastKind getCastKind() const { return castKind; }

    // static ImplicitKind inferArithCastKind(ArithType *from, ArithType *to);

private:
    Expr *fromExpr;
    CastKind castKind;
};

template <typename ResultTy = unsigned long long>
class IntegerExprEvaluator : public ExprVisitor {
public:
    IntegerExprEvaluator(const ResultTy &res) : result(res) {}

#define EXPR(CLASS) void visit(CLASS *e);
#include "ASTNode.def"

private:
    ResultTy &result;
};

/**
 * Declarations
 */
class Decl : public Node {
public:
    Decl(SourceLocation loc) : Node(loc) {}

    virtual void accept(DeclVisitor *v) = 0;
};

template <typename Derived>
class VisitableDecl : public Decl {
public:
    template <typename... Args>
    VisitableDecl(Args &&...args) : Decl(std::forward<Args>(args)...) {}

    void accept(DeclVisitor *v) { v->visit(static_cast<Derived *>(this)); }
};

class EmptyDecl : public VisitableDecl<EmptyDecl> {
public:
};

/**
 * FunctionDecl - For function declaration and definition.
 */
class FunctionDecl : public VisitableDecl<FunctionDecl> {
public:
    QualType prototype;
    std::vector<void *> paramVarDecls;
    CompoundStmt *stmts;
};

class VarDecl : public VisitableDecl<VarDecl> {
public:
    QualType type;
    IdentifierInfo *name;
};

/**
 * EnumDecl - Enumeration type declaration.
 * Since this compiler follows ISO C99 strictly,
 * forward reference to enum type is forbidden.
 */
class EnumDecl : public VisitableDecl<EnumDecl> {
public:
    EnumDecl(SourceLocation loc) : VisitableDecl<EnumDecl>(loc) {}

    QualType getIntegerType() const { return integerType; }

private:
    // integerType - The underlying integer type which defaults to int.
    // ISO C99 does not allow declaration with a fixed underlying type.
    QualType integerType;
};

class TypedefDecl : public VisitableDecl<TypedefDecl> {
public:
private:
};

class FieldDecl : public VisitableDecl<FieldDecl> {
public:
    FieldDecl(SourceLocation loc, QualType qt, const std::string &name = "", Expr *bitWidth = nullptr);

    void accept(Visitor *v) { v->visitFieldDecl(this); }

    QualType getType() const { return type; }
    // TODO: Bit-field width should be a const integer.
    // We should check when evaluating.
    std::size_t getBitWidthValue(ASTContext &ctx) const {
        std::size_t evalRet;
        bool result = bitWidth->evaluateAsUnsignedInt(evalRet, ctx);
        return result ? evalRet : 0ULL;
    }

    std::string fieldName;

private:
    QualType type;

    // bitWidth - Bit-field declaration, declares a member with explicit width, in bits.
    // Adjacent bit-field members may be packed to share and straddle the individual bytes.
    // If struct member is declared normally, bitWidth will be set to nullptr.
    Expr *bitWidth;
};

class RecordDecl : public VisitableDecl<RecordDecl> {
public:
    enum RecordKind {
        RK_STRUCT,
        RK_UNION,
    };

    RecordDecl(SourceLocation loc, RecordKind k, bool isDef = true, std::string name = "");

    void accept(Visitor *v) { v->visitRecordDecl(this); }

private:
    RecordKind recordDeclKind;
    // isDef - Whether this declaration is a definition.
    bool isDef;
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

    virtual void accept(StmtVisitor *v) = 0;
};

template <typename Derived>
class VisitableStmt : public Stmt {
public:
    template <typename... Args>
    VisitableStmt(Args &&...args) : Stmt(std::forward<Args>(args)...) {}

    void accept(StmtVisitor *v) { v->visit(static_cast<Derived *>(this)); }
};

class DeclStmt : public VisitableStmt<DeclStmt> {
public:
    std::vector<Decl *> decls;
};

class BreakStmt : public VisitableStmt<BreakStmt> {
public:
    BreakStmt(const SourceLocation &loc) : VisitableStmt<BreakStmt>(loc) {}
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

/**
 * AST context
 */
class ASTContext {
    std::vector<Type *> types;

public:
    // C builtin types.
    QualType voidTy;
    QualType boolTy;
    QualType charTy;
    QualType signedCharTy, shortTy, intTy, longTy, longLongTy;
    QualType unsignedCharTy, unsignedShortTy, unsignedIntTy, unsignedLongTy, unsignedLongLongTy;
    QualType floatTy, doubleTy, longDoubleTy;

    ASTContext();

    // getIntegerRank - [C99 6.3.1.1p1] Integer conversion rank.
    unsigned getIntegerRank(Type *t) const;
    unsigned getFloatingRank(Type *t) const;

    // [C99 6.3.1.1p2]
    QualType getPromotedIntegerType(QualType promotable);
    bool isPromotableIntegerType(QualType t) const;
    QualType isPromotableBitField(Expr *e);

    // getTypeSize - Return the size of the specified type, in bits.
    // This method does not work on incomplete types.
    std::size_t getTypeSize(QualType t) { return getTypeSize(t.getTypePtr()); }
    std::size_t getTypeSize(Type *t);

private:
    void initBuiltinTypes();
    void initVoidType(QualType &r);
    void initArithType(QualType &r, ArithType::ArithKind k);

    std::unordered_map<Type *, std::size_t> memorizedTypeSize;
};

/**
 * AST dumper
 */
class ASTDumper : public ExprVisitor,
                  public DeclVisitor,
                  public StmtVisitor {
public:
    ASTDumper(std::ostream &os) : out(os) {}

#define EXPR(CLASS) void visit(CLASS *e);
#include "ASTNode.def"

#define DECL(CLASS) void visit(CLASS *d);
#include "ASTNode.def"

    void visit(BreakStmt *breakStmt);

    // Helper functions.
    template <typename ASTNode>
    void dumpChild(ASTNode *node);

    template <typename ASTNode>
    void dumpLastChild(ASTNode *node);

private:
    // prefix - Indentation of every line.
    std::string prefix;
    std::ostream &out;
};

#endif
