#include "ast.h"
#include <cassert>

UnaryExpr::UnaryExpr(const SourceLocation &loc, UnaryOpKind op, Expr *expr)
    : Expr(loc, QualType()), op(op), operand(expr) {}

SizeofExpr::SizeofExpr(const SourceLocation &loc, Expr *expr)
    : Expr(loc, ArithType::getArithType(ArithType::UNSIGNED_LONG_LONG)) {
    sizeofKind = UNARY_EXPR;
    arg.expr = expr;
}

SizeofExpr::SizeofExpr(const SourceLocation &loc, QualType type)
    : Expr(loc, ArithType::getArithType(ArithType::UNSIGNED_LONG_LONG)) {
    sizeofKind = TYPE_NAME;
    arg.type = type;
}

BinaryExpr::BinaryExpr(const SourceLocation &loc, BinaryOpKind op, Expr *lhs, Expr *rhs)
    : Expr(loc, QualType()), op(op), lhs(lhs), rhs(rhs) {
}

void BinaryExpr::checkAdditiveOperator() {
    QualType lqt = lhs->qtype;
    QualType rqt = rhs->qtype;

    // TODO: Value transformations.
    // Lvalue-to-rvalue conversion.
    // Array-to-pointer conversion.
    // Function-to-pointer conversion.

    ArithType *l = dynamic_cast<ArithType *>(lqt.type);
    ArithType *r = dynamic_cast<ArithType *>(rqt.type);

    if (lqt.type->kind == ARITH && rqt.type->kind == ARITH) {
        ArithType *c = ArithType::usualArithConv(l, r);
        if (l != c) {
            ImplicitCastExpr::ImplicitKind castK = ImplicitCastExpr::inferArithCastKind(l, c);
            lhs = new ImplicitCastExpr(lhs->srcLoc, lhs, QualType(c), castK);
        }
        if (r != c) {
            ImplicitCastExpr::ImplicitKind castK = ImplicitCastExpr::inferArithCastKind(r, c);
            rhs = new ImplicitCastExpr(rhs->srcLoc, rhs, QualType(c), castK);
        }
        qtype.type = c;
    }
}

TernaryExpr::TernaryExpr(const SourceLocation &loc, Expr *condExpr, Expr *trueExpr, Expr *falseExpr)
    : Expr(loc, QualType()), condExpr(condExpr), trueExpr(trueExpr), falseExpr(falseExpr) {}

IntegerConstant::IntegerConstant(const SourceLocation &loc, const QualType &qt, unsigned long long val)
    : Expr(loc, qt), value(val) {}

FloatingConstant::FloatingConstant(const SourceLocation &loc, const QualType &qt, long double val)
    : Expr(loc, qt), value(val) {}

CharacterConstant::CharacterConstant(const SourceLocation &loc, const QualType &qt, unsigned val)
    : Expr(loc, qt), value(val) {}

StringLiteral::StringLiteral(const SourceLocation &loc, const QualType &qt, const std::string &str)
    : Expr(loc, qt), value(str) {}

ImplicitCastExpr::ImplicitCastExpr(const SourceLocation &loc, Expr *from, const QualType &to, ImplicitKind cKind)
    : Expr(loc, to), fromExpr(from), castKind(cKind) {}

ImplicitCastExpr::ImplicitKind ImplicitCastExpr::inferArithCastKind(ArithType *from, ArithType *to) {
    if (from->isInteger() && to->isInteger()) {
        return ImplicitCastExpr::INTEGRAL_CAST;
    } else if (from->isFloating() && to->isFloating()) {
        return ImplicitCastExpr::FLOATING_CAST;
    } else {
        return ImplicitCastExpr::INTEGRAL_TO_FLOATING;
    }
}

FieldDecl::FieldDecl(const SourceLocation &loc, QualType qt, const std::string &name, Expr *bitWidth, unsigned offset)
    : Decl(loc), type(qt), fieldName(name), bitWidth(bitWidth), offset(offset) {}

RecordDecl::RecordDecl(const SourceLocation &loc, bool isStruct, bool isDef, std::string name)
    : Decl(loc), isDef(isDef), isStruct(isStruct), recordName(name) {}

/**
 * AST context
 */
ASTContext::ASTContext() {
    initBuiltinTypes();
}

bool ASTContext::isPromotableIntegerType(QualType t) {
    if (const ArithType *bt = t->getAs<ArithType>()) {
        switch (bt->getArithKind()) {
        case ArithType::BOOL:
        case ArithType::CHAR_S:
        case ArithType::SIGNED_CHAR:
        case ArithType::UNSIGNED_CHAR:
        case ArithType::SHORT:
        case ArithType::UNSIGNED_SHORT:
            return true;
        default:
            return false;
        }
    }

    // Enumerated types are promotable to their compatible integer types
    // (C99 6.3.1.1) a.k.a. its underlying type.
    if (const EnumType *et = t->getAs<EnumType>()) {
        if (et->getDecl()->getPromotionType().isNull())
            return false;
        return true;
    }

    return false;
}

QualType ASTContext::isPromotableBitField(Expr *e) {
    // FieldDecl *field=
}

void ASTContext::initVoidType(QualType &r) {
    Type *t = VoidType::getVoidType();
    types.emplace_back(t);
    r = QualType(t);
}

void ASTContext::initArithType(QualType &r, ArithType::ArithKind k) {
    Type *t = ArithType::getArithType(k);
    types.emplace_back(t);
    r = QualType(t);
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
void ASTDumper::visitUnaryExpr(UnaryExpr *unary) {
    out << "UnaryExpr <" << srcLocToPos(unary->srcLoc) << "> ";
    switch (unary->op) {
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
    dumpLastChild(unary->operand);
}

void ASTDumper::visitSizeofExpr(SizeofExpr *sizeofExpr) {
    auto reprPair = sizeofExpr->getQualType().repr();
    out << "SizeofExpr <" << srcLocToPos(sizeofExpr->srcLoc) << "> '";
    out << reprPair.first << reprPair.second << "' sizeof ";
    if (sizeofExpr->sizeofKind == SizeofExpr::UNARY_EXPR) {
        out << '\n';
        dumpLastChild(sizeofExpr->arg.expr);
    } else if (sizeofExpr->sizeofKind == SizeofExpr::TYPE_NAME) {
        auto reprPair = sizeofExpr->arg.type.repr();
        out << '\'' << reprPair.first << reprPair.second << "'\n";
    }
}

void ASTDumper::visitBinaryExpr(BinaryExpr *binary) {
    auto reprPair = binary->getQualType().repr();
    out << "BinaryExpr <" << srcLocToPos(binary->srcLoc) << "> '";
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

void ASTDumper::visitTernaryExpr(TernaryExpr *ternary) {
    out << "TernaryExpr <" << srcLocToPos(ternary->srcLoc) << ">\n";
    dumpChild(ternary->condExpr);
    dumpChild(ternary->trueExpr);
    dumpLastChild(ternary->falseExpr);
}

void ASTDumper::visitIntegerConstant(IntegerConstant *integer) {
    auto reprPair = integer->getQualType().repr();
    out << "IntegerConstant <" << srcLocToPos(integer->srcLoc) << "> '";
    out << reprPair.first << reprPair.second << "' " << integer->value << '\n';
}

void ASTDumper::visitFloatingConstant(FloatingConstant *floating) {
    auto reprPair = floating->getQualType().repr();
    out << "FloatingConstant <" << srcLocToPos(floating->srcLoc) << "> '";
    out << reprPair.first << reprPair.second << "' " << floating->value << '\n';
}

void ASTDumper::visitCharacterConstant(CharacterConstant *character) {
    auto reprPair = character->getQualType().repr();
    out << "CharacterConstant <" << srcLocToPos(character->srcLoc) << "> '";
    out << reprPair.first << reprPair.second << "' " << character->value << '\n';
}

void ASTDumper::visitStringLiteral(StringLiteral *string) {
    auto reprPair = string->getQualType().repr();
    out << "StringLiteral <" << srcLocToPos(string->srcLoc) << "> '";
    out << reprPair.first << reprPair.second << "' \"" << string->value << "\"\n";
}

void ASTDumper::visitDeclRefExpr(DeclRefExpr *declRef) {
}

void ASTDumper::visitImplicitCastExpr(ImplicitCastExpr *implicitCast) {
    auto reprPair = implicitCast->getQualType().repr();
    out << "ImplicitCastExpr <" << srcLocToPos(implicitCast->srcLoc) << "> '";
    out << reprPair.first << reprPair.second << "' ";
    switch (implicitCast->castKind) {
    case ImplicitCastExpr::INTEGRAL_CAST:
        out << "<IntegralCast>\n";
        break;
    case ImplicitCastExpr::FLOATING_CAST:
        out << "<FloatingCast>\n";
        break;
    case ImplicitCastExpr::INTEGRAL_TO_FLOATING:
        out << "<IntegralToFloating>\n";
        break;
    case ImplicitCastExpr::LVALUE_TO_RVALUE:
        out << "<LValueToRValue>\n";
        break;
    case ImplicitCastExpr::ARRAY_DECAY:
        out << "<ArrayToPointerDecay>\n";
        break;
    case ImplicitCastExpr::FUNCTION_DECAY:
        out << "<FunctionToPointerDecay>\n";
        break;
    }
    dumpLastChild(implicitCast->fromExpr);
}

void ASTDumper::visitVarDecl(VarDecl *varDecl) {
}

void ASTDumper::visitFieldDecl(FieldDecl *fieldDecl) {
    auto reprPair = fieldDecl->type.repr();
    out << "FieldDecl <" << srcLocToPos(fieldDecl->srcLoc) << "> ";
    out << fieldDecl->fieldName << " '";
    out << reprPair.first << reprPair.second << "' \n";
}

void ASTDumper::visitRecordDecl(RecordDecl *recordDecl) {
    out << "RecordDecl <" << srcLocToPos(recordDecl->srcLoc) << "> ";
    out << "struct " << recordDecl->recordName;
    if (recordDecl->isDef)
        out << " definition\n";
    else
        out << '\n';
    for (size_t i = 0, e = recordDecl->fields.size(); i < e - 1; ++i) {
        dumpChild(recordDecl->fields[i]);
    }
    dumpLastChild(recordDecl->fields.back());
}

void ASTDumper::visitBreakStmt(BreakStmt *breakStmt) {
    out << "BreakStmt <" << srcLocToPos(breakStmt->srcLoc) << ">\n";
}

void ASTDumper::dumpChild(Node *node) {
    out << prefix << "|--";
    prefix.append("|  ");
    node->accept(this);
    prefix.erase(prefix.size() - 3);
}

void ASTDumper::dumpLastChild(Node *node) {
    out << prefix << "`--";
    prefix.append("   ");
    node->accept(this);
    prefix.erase(prefix.size() - 3);
}