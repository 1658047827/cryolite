#include "semantic.h"
#include "diagnostic.h"
#include <sstream>

void Semantic::actOnPopScope(SourceLocation loc, Scope *s) {
    if (s->declEmpty()) return;
    assert((s->getFlags() & Scope::DECL_SCOPE) && "Scope shouldn't contain decls");
}

void Semantic::defaultFunctionArrayConversion(Expr *&expr) {
}

Expr *Semantic::usualUnaryConversions(Expr *&expr) {
    QualType ty = expr->getQualType();
    assert(!ty.isNull() && "missing type in usual unary conversions");

    // [C99 6.3.1.1p2] Integer promotion.
}

// usualArithmeticConversions - [C99 6.3.1.8]
QualType Semantic::usualArithmeticConversions(Expr *lhs, Expr *rhs) {
    usualUnaryConversions(lhs);
    usualUnaryConversions(rhs);
}

Expr *Semantic::actOnBinaryOp(Scope *s, SourceLocation loc, BinaryOpKind op, Expr *lhs, Expr *rhs) {
    assert(lhs && "missing left expression");
    assert(rhs && "missing right expression");

    // Emit warnings for tricky precedence issues, e.g. "bitfield & 0x4 == 0"
    if (BinaryExpr::isBitwiseOp(op))
        diagnoseBitwisePrecedence(op, loc, lhs, rhs);

    return buildBinaryExpr(s, loc, op, lhs, rhs);
}

Expr *Semantic::buildBinaryExpr(Scope *s, SourceLocation loc, BinaryOpKind op, Expr *lhs, Expr *rhs) {
    QualType resultTy;

    switch (op) {
    case BinaryOpKind::ADD:
        break;
    case BinaryOpKind::SUB:
        break;
    case BinaryOpKind::MUL:
    case BinaryOpKind::DIV:
    case BinaryOpKind::MOD:
        break;
    default:
        assert(0);
    }
}

QualType Semantic::checkAdditionOperands(Expr *lhs, Expr *rhs, SourceLocation loc) {
}

static void diagnoseBitwisePrecedence(BinaryOpKind opK, SourceLocation opLoc, Expr *lhs, Expr *rhs) {
    BinaryExpr *lhsBe = dynamic_cast<BinaryExpr *>(lhs);
    BinaryExpr *rhsBe = dynamic_cast<BinaryExpr *>(rhs);

    bool isLeftComp = lhsBe && lhsBe->isComparisonOp();
    bool isRightComp = rhsBe && rhsBe->isComparisonOp();
    if (isLeftComp == isRightComp)
        return;

    // Bitwise operations are sometimes used as eager logical ops.
    // Don't diagnose this.
    bool isLeftBitwise = lhsBe && lhsBe->isBitwiseOp();
    bool isRightBitwise = rhsBe && rhsBe->isBitwiseOp();
    if (isLeftBitwise || isRightBitwise)
        return;

    std::stringstream ss;
    std::string_view cmpOpStr = isLeftComp ? lhsBe->getOpStr() : rhsBe->getOpStr();
    ss << BinaryExpr::getOpStr(opK) << " has lower precedence than ";
    ss << cmpOpStr << ", " << cmpOpStr << " will be evaluated first";
    warning(opLoc, ss.str());
}