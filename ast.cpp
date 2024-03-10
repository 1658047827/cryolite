#include "ast.h"
#include <cassert>

bool Expr::evaluateAsUnsignedInt(std::size_t &evalRet, ASTContext &ctx) const {
    return true;
}

bool Expr::evaluateAsFloating(long double &evalRet, ASTContext &ctx) const {
    return true;
}

UnaryExpr::UnaryExpr(SourceLocation loc, UnaryOpKind op, Expr *expr)
    : VisitableExpr<UnaryExpr>(loc, QualType()), op(op), operand(expr) {}

SizeofExpr::SizeofExpr(SourceLocation loc, QualType resultTy, Expr *expr)
    : VisitableExpr<SizeofExpr>(loc, resultTy) {
    sizeofKind = UNARY_EXPR;
    arg.expr = expr;
}

SizeofExpr::SizeofExpr(SourceLocation loc, QualType resultTy, QualType type)
    : VisitableExpr<SizeofExpr>(loc, resultTy) {
    sizeofKind = TYPE_NAME;
    arg.type = type;
}

BinaryExpr::BinaryExpr(SourceLocation loc, BinaryOpKind op, Expr *lhs, Expr *rhs)
    : VisitableExpr<BinaryExpr>(loc, QualType()), op(op), lhs(lhs), rhs(rhs) {
}

std::string_view BinaryExpr::getOpStr(BinaryOpKind opK) {
    switch (opK) {
#define BINARY(NAME, REPR) \
    case NAME:             \
        return REPR;
#include "operatorKind.def"
    }
    assert(0 && "invalid binary operator");
}

// void BinaryExpr::checkAdditiveOperator() {
//     QualType lqt = lhs->qtype;
//     QualType rqt = rhs->qtype;

//     // TODO: Value transformations.
//     // Lvalue-to-rvalue conversion.
//     // Array-to-pointer conversion.
//     // Function-to-pointer conversion.

//     ArithType *l = dynamic_cast<ArithType *>(lqt.type);
//     ArithType *r = dynamic_cast<ArithType *>(rqt.type);

//     if (lqt.type->kind == ARITH && rqt.type->kind == ARITH) {
//         ArithType *c = ArithType::usualArithConv(l, r);
//         if (l != c) {
//             ImplicitCastExpr::ImplicitKind castK = ImplicitCastExpr::inferArithCastKind(l, c);
//             lhs = new ImplicitCastExpr(lhs->srcLoc, lhs, QualType(c), castK);
//         }
//         if (r != c) {
//             ImplicitCastExpr::ImplicitKind castK = ImplicitCastExpr::inferArithCastKind(r, c);
//             rhs = new ImplicitCastExpr(rhs->srcLoc, rhs, QualType(c), castK);
//         }
//         qtype.type = c;
//     }
// }

TernaryExpr::TernaryExpr(SourceLocation loc, Expr *condExpr, Expr *trueExpr, Expr *falseExpr)
    : VisitableExpr<TernaryExpr>(loc, QualType()), condExpr(condExpr), trueExpr(trueExpr), falseExpr(falseExpr) {}

IntegerConstant::IntegerConstant(SourceLocation loc, QualType qt, unsigned long long ullVal)
    : VisitableExpr<IntegerConstant>(loc, qt), ullValue(ullVal) {}

IntegerConstant::IntegerConstant(SourceLocation loc, QualType qt, signed long long sllVal)
    : VisitableExpr<IntegerConstant>(loc, qt), sllValue(sllVal) {}

FloatingConstant::FloatingConstant(SourceLocation loc, QualType qt, long double val)
    : VisitableExpr<FloatingConstant>(loc, qt), value(val) {}

CharacterConstant::CharacterConstant(SourceLocation loc, QualType qt, int val)
    : VisitableExpr<CharacterConstant>(loc, qt), value(val) {}

StringLiteral::StringLiteral(SourceLocation loc, QualType qt, const std::string &str)
    : VisitableExpr<StringLiteral>(loc, qt), content(str) {}

ImplicitCastExpr::ImplicitCastExpr(Expr *from, QualType to, CastKind cKind)
    : VisitableExpr<ImplicitCastExpr>(from->getSrcLoc(), to), fromExpr(from), castKind(cKind) {}

// ImplicitCastExpr::ImplicitKind ImplicitCastExpr::inferArithCastKind(ArithType *from, ArithType *to) {
//     if (from->isInteger() && to->isInteger()) {
//         return ImplicitCastExpr::INTEGRAL_CAST;
//     } else if (from->isFloating() && to->isFloating()) {
//         return ImplicitCastExpr::FLOATING_CAST;
//     } else {
//         return ImplicitCastExpr::INTEGRAL_TO_FLOATING;
//     }
// }

ParenExpr::ParenExpr(SourceLocation loc, Expr *subExpr)
    : VisitableExpr<ParenExpr>(loc, QualType()), subExpr(subExpr) {}

template <typename ResultTy>
void IntegerExprEvaluator<ResultTy>::visit(UnaryExpr *unary) {}

template <typename ResultTy>
void IntegerExprEvaluator<ResultTy>::visit(SizeofExpr *sizeofExpr) {}

template <typename ResultTy>
void IntegerExprEvaluator<ResultTy>::visit(BinaryExpr *binary) {}

template <typename ResultTy>
void IntegerExprEvaluator<ResultTy>::visit(TernaryExpr *ternary) {}

template <typename ResultTy>
void IntegerExprEvaluator<ResultTy>::visit(IntegerConstant *integer) {}

template <typename ResultTy>
void IntegerExprEvaluator<ResultTy>::visit(FloatingConstant *floating) {}

template <typename ResultTy>
void IntegerExprEvaluator<ResultTy>::visit(CharacterConstant *character) {}

template <typename ResultTy>
void IntegerExprEvaluator<ResultTy>::visit(StringLiteral *string) {}

template <typename ResultTy>
void IntegerExprEvaluator<ResultTy>::visit(DeclRefExpr *declRef) {}

template <typename ResultTy>
void IntegerExprEvaluator<ResultTy>::visit(ParenExpr *paren) {}

template <typename ResultTy>
void IntegerExprEvaluator<ResultTy>::visit(CallExpr *call) {}

template <typename ResultTy>
void IntegerExprEvaluator<ResultTy>::visit(MemberExpr *member) {}

template <typename ResultTy>
void IntegerExprEvaluator<ResultTy>::visit(CastExpr *cast) {}

template <typename ResultTy>
void IntegerExprEvaluator<ResultTy>::visit(ImplicitCastExpr *implicitCast) {}

FieldDecl::FieldDecl(SourceLocation loc, QualType qt, const std::string &name, Expr *bitWidth)
    : VisitableDecl<FieldDecl>(loc), type(qt), fieldName(name), bitWidth(bitWidth) {}

RecordDecl::RecordDecl(SourceLocation loc, RecordKind k, bool isDef, std::string name)
    : VisitableDecl<RecordDecl>(loc), isDef(isDef), recordDeclKind(k), recordName(name) {}

/**
 * AST context
 */
ASTContext::ASTContext() {
    initBuiltinTypes();
}

QualType ASTContext::getPromotedIntegerType(QualType promotable) {
    if (promotable->isSignedIntegerType())
        return intTy;
    std::size_t promotableSize = getTypeSize(promotable);
    std::size_t intTypeSize = getTypeSize(intTy);
    assert(promotable->isUnsignedIntegerType() && promotableSize <= intTypeSize);
    return (promotableSize != intTypeSize) ? intTy : unsignedIntTy;
}

bool ASTContext::isPromotableIntegerType(QualType t) const {
    if (const ArithType *bt = t->getAs<ArithType>()) {
        switch (bt->getArithKind()) {
        case ArithType::BOOL:
        case ArithType::CHAR_S:
        case ArithType::CHAR_U:
        case ArithType::SIGNED_CHAR:
        case ArithType::UNSIGNED_CHAR:
        case ArithType::SHORT:
        case ArithType::UNSIGNED_SHORT:
            return true;
        default:
            return false;
        }
    }
    // In this compiler, enum's underlying type is int.
    // So enum is not promotable.
    return false;
}

QualType ASTContext::isPromotableBitField(Expr *e) {
    // TODO: Finish it.

    // FieldDecl *field =

    // std::size_t bitWidth =
    std::size_t intSize = 0;

    return QualType();
}

unsigned ASTContext::getIntegerRank(Type *t) const {
    assert(t->isCanonicalUnqualified() && "t should be canonicalized");
    if (const auto *et = dynamic_cast<EnumType *>(t))
        t = et->getDecl()->getIntegerType().getTypePtr();
    assert(t->isArithmeticType());
    switch (t->getAs<ArithType>()->getArithKind()) {
    case ArithType::BOOL:
        return 1;
    case ArithType::CHAR_S:
    case ArithType::CHAR_U:
    case ArithType::SIGNED_CHAR:
    case ArithType::UNSIGNED_CHAR:
        return 2;
    case ArithType::SHORT:
    case ArithType::UNSIGNED_SHORT:
        return 3;
    case ArithType::INT:
    case ArithType::UNSIGNED_INT:
        return 4;
    case ArithType::LONG:
    case ArithType::UNSIGNED_LONG:
        return 5;
    case ArithType::LONG_LONG:
    case ArithType::UNSIGNED_LONG_LONG:
        return 6;
    default:
        assert(0 && "not an integer type");
    }
    return 0;
}

unsigned ASTContext::getFloatingRank(Type *t) const {
    assert(t->isArithmeticType());
    switch (t->getAs<ArithType>()->getArithKind()) {
    case ArithType::FLOAT:
        return 7;
    case ArithType::DOUBLE:
        return 8;
    case ArithType::LONG_DOUBLE:
        return 9;
    default:
        assert(0 && "not a floating type");
    }
}

void ASTContext::initVoidType(QualType &r) {
    Type *t = new VoidType();
    types.emplace_back(t);
    r = QualType(t);
}

void ASTContext::initArithType(QualType &r, ArithType::ArithKind k) {
    Type *t = new ArithType(k);
    types.emplace_back(t);
    r = QualType(t);
}

std::size_t ASTContext::getTypeSize(Type *t) {
    auto iter = memorizedTypeSize.find(t);
    if (iter != memorizedTypeSize.end())
        return iter->second;

    std::size_t width = 0;

    switch (t->getKind()) {
    case TypeKind::VOID:
        width = 0;
        break;
    case TypeKind::ARITH: {
        switch (t->getAs<ArithType>()->getArithKind()) {
#define ARITH(T, BITSIZE, REPR) \
    case ArithType::T:          \
        width = BITSIZE;        \
        break;
#include "arithType.def"
        default:
            // unreachable
            assert(0 && "unknown arithmetic type");
        }
    }
    default:
        break;
    }

    memorizedTypeSize[t] = width;
    return width;
}

void ASTContext::initBuiltinTypes() {
    // void type
    initVoidType(voidTy);

    initArithType(boolTy, ArithType::BOOL);
    // TODO: Determine whether char is signed or unsigned.
    initArithType(charTy, ArithType::CHAR_S);

    initArithType(signedCharTy, ArithType::SIGNED_CHAR);
    initArithType(shortTy, ArithType::SHORT);
    initArithType(intTy, ArithType::INT);
    initArithType(longTy, ArithType::LONG);
    initArithType(longLongTy, ArithType::LONG_LONG);

    initArithType(unsignedCharTy, ArithType::UNSIGNED_CHAR);
    initArithType(unsignedShortTy, ArithType::UNSIGNED_SHORT);
    initArithType(unsignedIntTy, ArithType::UNSIGNED_INT);
    initArithType(unsignedLongTy, ArithType::UNSIGNED_LONG);
    initArithType(unsignedLongLongTy, ArithType::UNSIGNED_LONG_LONG);

    initArithType(floatTy, ArithType::FLOAT);
    initArithType(doubleTy, ArithType::DOUBLE);
    initArithType(longDoubleTy, ArithType::LONG_DOUBLE);
}

/**
 * AST dumper
 */
void ASTDumper::visit(UnaryExpr *unary) {
    out << "UnaryExpr <" << srcLocToPos(unary->getSrcLoc()) << "> ";
    switch (unary->getOp()) {
    case PREINC:
        out << "prefix '++'\n";
        break;
    case PREDEC:
        out << "prefix '--'\n";
        break;
    case POSINC:
        out << "postfix '++'\n";
        break;
    case POSDEC:
        out << "postfix '--'\n";
        break;
    case PLUS:
        out << "prefix '+'\n";
        break;
    case MINUS:
        out << "prefix '-'\n";
        break;
    case BITNOT:
        out << "prefix '~'\n";
        break;
    case LOGICNOT:
        out << "prefix '!'\n";
        break;
    case ADDR:
        out << "prefix '&'\n";
        break;
    case DEREF:
        out << "prefix '*'\n";
        break;
    default:
        break;
    }
    dumpLastChild(unary->getOperand());
}

void ASTDumper::visit(SizeofExpr *sizeofExpr) {
    auto reprPair = sizeofExpr->getQualType().repr();
    out << "SizeofExpr <" << srcLocToPos(sizeofExpr->getSrcLoc()) << "> '";
    out << reprPair.first << reprPair.second << "' sizeof ";
    if (sizeofExpr->sizeofKind == SizeofExpr::UNARY_EXPR) {
        out << '\n';
        dumpLastChild(sizeofExpr->arg.expr);
    } else if (sizeofExpr->sizeofKind == SizeofExpr::TYPE_NAME) {
        auto reprPair = sizeofExpr->arg.type.repr();
        out << '\'' << reprPair.first << reprPair.second << "'\n";
    }
}

void ASTDumper::visit(BinaryExpr *binary) {
    auto reprPair = binary->getQualType().repr();
    out << "BinaryExpr <" << srcLocToPos(binary->getSrcLoc()) << "> '";
    out << reprPair.first << reprPair.second << "' ";
    switch (binary->op) {
    case ADD:
        out << "'+'\n";
        break;
    case SUB:
        out << "'-'\n";
        break;
    case MUL:
        out << "'*'\n";
        break;
    case DIV:
        out << "'/'\n";
        break;
    case MOD:
        out << "'%'\n";
        break;
    case SHL:
        out << "'<<'\n";
        break;
    case SHR:
        out << "'>>'\n";
        break;
    case LESS:
        out << "'<'\n";
        break;
    case LEQ:
        out << "'<='\n";
        break;
    case GREATER:
        out << "'>'\n";
        break;
    case GEQ:
        out << "'>='\n";
        break;
    case EQUAL:
        out << "'=='\n";
        break;
    case NEQ:
        out << "'!='\n";
        break;
    case BITAND:
        out << "'&'\n";
        break;
    case BITXOR:
        out << "'^'\n";
        break;
    case BITOR:
        out << "'|'\n";
        break;
    case LOGICAND:
        out << "'&&'\n";
        break;
    case LOGICOR:
        out << "'||'\n";
        break;
    case ASSIGN:
        out << "'='\n";
        break;
    case COMMA:
        out << "','\n";
        break;
    default:
        break;
    }
    dumpChild(binary->lhs);
    dumpLastChild(binary->rhs);
}

void ASTDumper::visit(TernaryExpr *ternary) {
    out << "TernaryExpr <" << srcLocToPos(ternary->getSrcLoc()) << ">\n";
    dumpChild(ternary->condExpr);
    dumpChild(ternary->trueExpr);
    dumpLastChild(ternary->falseExpr);
}

void ASTDumper::visit(IntegerConstant *integer) {
    auto reprPair = integer->getQualType().repr();
    out << "IntegerConstant <" << srcLocToPos(integer->getSrcLoc()) << "> '";
    out << reprPair.first << reprPair.second << "' ";
    if (integer->getQualType()->isSignedIntegerType())
        out << integer->getSignedvalue() << '\n';
    else
        out << integer->getUnsignedValue() << '\n';
}

void ASTDumper::visit(FloatingConstant *floating) {
    auto reprPair = floating->getQualType().repr();
    out << "FloatingConstant <" << srcLocToPos(floating->getSrcLoc()) << "> '";
    out << reprPair.first << reprPair.second << "' " << floating->value << '\n';
}

void ASTDumper::visit(CharacterConstant *character) {
    auto reprPair = character->getQualType().repr();
    out << "CharacterConstant <" << srcLocToPos(character->getSrcLoc()) << "> '";
    out << reprPair.first << reprPair.second << "' " << character->value << '\n';
}

void ASTDumper::visit(StringLiteral *string) {
    auto reprPair = string->getQualType().repr();
    out << "StringLiteral <" << srcLocToPos(string->getSrcLoc()) << "> '";
    out << reprPair.first << reprPair.second << "' \"" << string->content << "\"\n";
}

void ASTDumper::visit(DeclRefExpr *declRef) {}

void ASTDumper::visit(ParenExpr *paren) {}

void ASTDumper::visit(CallExpr *call) {}

void ASTDumper::visit(MemberExpr *member) {}

void ASTDumper::visit(CastExpr *cast) {}

void ASTDumper::visit(ImplicitCastExpr *implicitCast) {
    auto reprPair = implicitCast->getQualType().repr();
    out << "ImplicitCastExpr <" << srcLocToPos(implicitCast->getSrcLoc()) << "> '";
    out << reprPair.first << reprPair.second << "' ";
    switch (implicitCast->getCastKind()) {
    case CastKind::INTEGRAL_CAST:
        out << "<IntegralCast>\n";
        break;
    case CastKind::FLOATING_CAST:
        out << "<FloatingCast>\n";
        break;
    case CastKind::INTEGRAL_TO_FLOATING:
        out << "<IntegralToFloating>\n";
        break;
    case CastKind::LVALUE_TO_RVALUE:
        out << "<LValueToRValue>\n";
        break;
    case CastKind::ARRAY_TO_POINTER_DECAY:
        out << "<ArrayToPointerDecay>\n";
        break;
    case CastKind::FUNCTION_TO_POINTER_DECAY:
        out << "<FunctionToPointerDecay>\n";
        break;
    }
    dumpLastChild(implicitCast->getFromExpr());
}

void ASTDumper::visit(EmptyDecl *emptyDecl) {}

void ASTDumper::visit(FunctionDecl *functionDecl) {}

void ASTDumper::visit(VarDecl *varDecl) {}

void ASTDumper::visit(EnumDecl *enumDecl) {}

void ASTDumper::visit(TypedefDecl *typedefDecl) {}

void ASTDumper::visit(FieldDecl *fieldDecl) {}

void ASTDumper::visit(RecordDecl *recordDecl) {}

void ASTDumper::visit(BreakStmt *breakStmt) {}

template <typename ASTNode>
void ASTDumper::dumpChild(ASTNode *node) {
    out << prefix << "|--";
    prefix.append("|  ");
    node->accept(this);
    prefix.erase(prefix.size() - 3);
}

template <typename ASTNode>
void ASTDumper::dumpLastChild(ASTNode *node) {
    out << prefix << "`--";
    prefix.append("   ");
    node->accept(this);
    prefix.erase(prefix.size() - 3);
}