#ifndef _CRYOLITE_AST_H_
#define _CRYOLITE_AST_H_

#include "token.h"
#include "type.h"
#include "visitor.h"
#include <ostream>
#include <vector>

class Decl;
class CompoundStmt;

class ASTContext;

class Node {
public:
    Node(const SourceLocation &loc) : srcLoc(loc) {}

    SourceLocation srcLoc;
};

/**
 * Expressions
 */
class Expr : public Node {
public:
    Expr(const SourceLocation &loc, const QualType &qt) : Node(loc), qtype(qt) {}

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
    VisitableExpr(const SourceLocation &loc, const QualType &qt) : Expr(loc, qt) {}

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
    UnaryExpr(const SourceLocation &loc, UnaryOpKind op, Expr *expr);

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
};

enum BinaryOpKind {
#define BINARY(NAME, REPR) NAME,
#include "operatorKind.def"
};

class BinaryExpr : public VisitableExpr<BinaryExpr> {
public:
    BinaryExpr(const SourceLocation &loc, BinaryOpKind op, Expr *lhs, Expr *rhs);

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
    TernaryExpr(const SourceLocation &loc, Expr *condExpr, Expr *trueExpr, Expr *falseExpr);

    Expr *condExpr;
    Expr *trueExpr;
    Expr *falseExpr;
};

class IntegerConstant : public VisitableExpr<IntegerConstant> {
public:
    IntegerConstant(const SourceLocation &loc, const QualType &qt, unsigned long long ullVal);
    IntegerConstant(const SourceLocation &loc, const QualType &qt, signed long long sllVal);

    unsigned long long getUnsignedValue() const { return ullValue; }
    signed long long getSignedvalue() const { return sllValue; }

private:
    union {
        unsigned long long ullValue;
        signed long long sllValue;
    };
    // Its sign can be read from qtype.
};

class FloatingConstant : public VisitableExpr<FloatingConstant> {
public:
    FloatingConstant(const SourceLocation &loc, const QualType &qt, long double val);

    long double value;
};

class CharacterConstant : public VisitableExpr<CharacterConstant> {
public:
    CharacterConstant(const SourceLocation &loc, const QualType &qt, unsigned val);

    unsigned value;
};

class StringLiteral : public VisitableExpr<StringLiteral> {
public:
    StringLiteral(const SourceLocation &loc, const QualType &qt, const std::string &str);

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

class CallExpr : public VisitableExpr<CallExpr> {
};

class MemberExpr : public VisitableExpr<MemberExpr> {
};

class CastExpr : public VisitableExpr<CastExpr> {
};

class ImplicitCastExpr : public VisitableExpr<ImplicitCastExpr> {
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

    Expr *getFromExpr() const { return fromExpr; }
    ImplicitKind getCastKind() const { return castKind; }

    // static ImplicitKind inferArithCastKind(ArithType *from, ArithType *to);

private:
    Expr *fromExpr;
    ImplicitKind castKind;
};

template <typename ResultTy = unsigned long long>
class IntegerExprEvaluator : public ExprVisitor {
public:
    IntegerExprEvaluator(const ResultTy &res) : result(res) {}

    void visit(UnaryExpr *unary);
    void visit(SizeofExpr *sizeofExpr);
    void visit(BinaryExpr *binary);
    void visit(TernaryExpr *ternary);
    void visit(IntegerConstant *integer);
    void visit(FloatingConstant *floating);
    void visit(CharacterConstant *character);
    void visit(StringLiteral *string);
    void visit(DeclRefExpr *declRef);
    void visit(CallExpr *call);
    void visit(MemberExpr *member);
    void visit(CastExpr *cast);
    void visit(ImplicitCastExpr *implicitCast);

private:
    ResultTy &result;
};

/**
 * Declarations
 */
class Decl : public Node {
public:
    Decl(const SourceLocation &loc) : Node(loc) {}

    virtual void accept(DeclVisitor *v) = 0;
};

template <typename Derived>
class VisitableDecl : public Decl {
public:
    VisitableDecl(const SourceLocation &loc) : Decl(loc) {}

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
    std::string name;
};

class EnumDecl : public VisitableDecl<EnumDecl> {
public:
    EnumDecl(const SourceLocation &loc) : VisitableDecl(loc) {}

    QualType getIntegerType() const { return integerType; }
    QualType getPromotionType() const { return promotionType; }

private:
    // integerType - The underlying integer type.
    QualType integerType;

    // promotionType - The integer type that this enum should promote to.
    QualType promotionType;
};

class TypedefDecl : public VisitableDecl<TypedefDecl> {
};

class FieldDecl : public VisitableDecl<FieldDecl> {
public:
    FieldDecl(const SourceLocation &loc, QualType qt, const std::string &name = "", Expr *bitWidth = nullptr);

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
    RecordDecl(const SourceLocation &loc, bool isStruct = true, bool isDef = true, std::string name = "");

    void accept(Visitor *v) { v->visitRecordDecl(this); }

    // isStruct - True if this record is a struct, false if this record is an union.
    bool isStruct;
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
    VisitableStmt(const SourceLocation &loc) : Stmt(loc) {}

    void accept(StmtVisitor *v) { v->visit(static_cast<Derived *>(this)); }
};

class DeclStmt : public VisitableStmt<DeclStmt> {
public:
    std::vector<Decl *> decls;
};

class BreakStmt : public VisitableStmt<BreakStmt> {
public:
    BreakStmt(const SourceLocation &loc) : VisitableStmt(loc) {}
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

    bool isPromotableIntegerType(QualType t);
    QualType isPromotableBitField(Expr *e);

private:
    void initBuiltinTypes();
    void initVoidType(QualType &r);
    void initArithType(QualType &r, ArithType::ArithKind k);
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
#include "exprNode.def"

#define DECL(CLASS) void visit(CLASS *d);
#include "declNode.def"

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
