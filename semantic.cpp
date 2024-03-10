#include "semantic.h"
#include "diagnostic.h"
#include <sstream>

void Semantic::actOnPopScope(SourceLocation loc, Scope *s) {
    if (s->declEmpty()) return;
    assert((s->getFlags() & Scope::DECL_SCOPE) && "Scope shouldn't contain decls");
}

Expr *Semantic::implicitCastExprToType(Expr *expr, QualType ty, CastKind cKind) {
    QualType exprTy = expr->getQualType();
    if (exprTy == ty)
        return expr;
    return new ImplicitCastExpr(expr, ty, cKind);
}

Expr *Semantic::lvalueConversion(Expr *expr) {
    // TODO: Finish it.
    return expr;
}

Expr *Semantic::functionArrayConversion(Expr *expr) {
}

Expr *Semantic::integerPromotions(Expr *expr) {
    QualType pTy = context.isPromotableBitField(expr);
    if (!pTy.isNull()) {
        expr = implicitCastExprToType(expr, pTy, CastKind::INTEGRAL_CAST);
        return expr;
    }
    QualType ty = expr->getQualType();
    if (context.isPromotableIntegerType(ty)) {
        QualType pt = context.getPromotedIntegerType(ty);
        expr = implicitCastExprToType(expr, pt, CastKind::INTEGRAL_CAST);
        return expr;
    }
}

// usualArithmeticConversions - [C99 6.3.1.8]
QualType Semantic::usualArithmeticConversions(Expr *&lhs, Expr *&rhs) {
    // For conversion purposes, we ignore any qualifiers.
    QualType lhsTy = lhs->getQualType().getUnqualifiedType();
    QualType rhsTy = rhs->getQualType().getUnqualifiedType();
    assert(lhsTy->isArithmeticType() && rhsTy->isArithmeticType());

    if (lhsTy->isFloatingType() || rhsTy->isFloatingType())
        handleFloatingConversions(lhs, rhs, lhsTy, rhsTy);
}

QualType Semantic::handleFloatingConversions(Expr *&lhs, Expr *&rhs, QualType lhsTy, QualType rhsTy) {
    bool lhsFloating = lhsTy->isFloatingType();
    bool rhsFloating = rhsTy->isFloatingType();

    if (lhsFloating && rhsFloating) {
    }
}

Expr *Semantic::actOnBinaryOp(Scope *s, SourceLocation opLoc, BinaryOpKind op, Expr *lhs, Expr *rhs) {
    assert(lhs && "missing left expression");
    assert(rhs && "missing right expression");

    // Emit warnings for tricky precedence issues, e.g.
    //     0x1 & 0x4 == 0
    //     1 << 2 + 4
    if (BinaryExpr::isBitwiseOp(op))
        diagnoseBitwisePrecedence(op, opLoc, lhs, rhs);
    if (op == SHL || op == SHR)
        diagnoseAdditiveInShift(op, opLoc, lhs, rhs);

    return createBuiltinBinaryExpr(s, opLoc, op, lhs, rhs);
}

Expr *Semantic::createBuiltinBinaryExpr(Scope *s, SourceLocation opLoc, BinaryOpKind op, Expr *lhs, Expr *rhs) {
    QualType resultTy;

    switch (op) {
    case BinaryOpKind::ADD:
        resultTy = checkAdditionOperands(lhs, rhs, opLoc);
        break;
    case BinaryOpKind::SUB:
        break;
    case BinaryOpKind::MUL:
    case BinaryOpKind::DIV:
    case BinaryOpKind::MOD:
        break;
    default:
        // assert(0);
        break;
    }
    return new BinaryExpr(opLoc, op, lhs, rhs);
}

QualType Semantic::checkAdditionOperands(Expr *&lhs, Expr *&rhs, SourceLocation loc) {
    QualType lhsTy = lhs->getQualType();
    QualType rhsTy = rhs->getQualType();

    if (lhsTy->isArithmeticType() && rhsTy->isArithmeticType()) {
        QualType type = usualArithmeticConversions(lhs, rhs);
        if (!type.isNull() && type->isArithmeticType())
            return type;
        // TODO: else?
    }

    // Diagnose string literal + int and string + char literal.

    // TODO: Other situations.
    return QualType();
}

Expr *Semantic::actOnNumericConstant(const Token &tok) {
    std::string s = tok.getStr();
    NumericLiteralParser numeric(s.data(), s.data() + s.size(), tok.getLoc());

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

static void diagnoseAdditiveInShift(BinaryOpKind opK, SourceLocation opLoc, Expr *lhs, Expr *rhs) {
    BinaryExpr *lhsBe = dynamic_cast<BinaryExpr *>(lhs);
    BinaryExpr *rhsBe = dynamic_cast<BinaryExpr *>(rhs);

    bool isLeftAdditive = lhsBe && lhsBe->isAdditiveOp();
    bool isRightAdditive = rhsBe && rhsBe->isAdditiveOp();
    if (isLeftAdditive == isRightAdditive)
        return;

    std::stringstream ss;
    std::string_view addOpStr = isLeftAdditive ? lhsBe->getOpStr() : rhsBe->getOpStr();
    ss << BinaryExpr::getOpStr(opK) << " has lower precedence than ";
    ss << addOpStr << ", " << addOpStr << " will be evaluated first";
    warning(opLoc, ss.str());
}