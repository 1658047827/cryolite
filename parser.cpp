#include "parser.h"
#include "diagnostic.h"
#include <cassert>

inline bool Parser::isKind(TokenSeqConstIter &iter, TokenKind kind) {
    return (*iter)->getKind() == kind;
}

void Parser::initBitSet() {
    setBitSet(firstOfSpecifierQualifier,
              // TypeQualifier
              TK_CONST,
              TK_RESTRICT,
              TK_VOLATILE,
              // TypeSpecifier
              TK_VOID,
              TK_CHAR,
              TK_SHORT,
              TK_INT,
              TK_LONG,
              TK_FLOAT,
              TK_DOUBLE,
              TK_SIGNED,
              TK_UNSIGNED,
              TK_STRUCT,
              TK_UNION,
              TK_ENUM,
              TK_IDENTIFIER);

    setBitSet(firstOfDeclarator,
              // Pointer
              TK_STAR,
              // DirectDeclarator
              TK_IDENTIFIER,
              TK_LPAR);
}

bool Parser::isInBitSet(TokenBitSet &bitset, TokenSeqConstIter &iter) {
    return bitset[(*iter)->getKind()];
}

bool Parser::isFirstOfTypeName(TokenSeqConstIter &iter) {
    return isInBitSet(firstOfSpecifierQualifier, iter);
}

TransUnit *Parser::parseTranslationUnit() {
    auto begin = cursor;
    TranslationUnit *transUnit = new TranslationUnit;
    while (!isKind(cursor, TK_EOF)) {
        // ; null statement
        if (isKind(cursor, TK_SEMI)) {
            ++cursor;
            continue;
        }
        auto externDecl = parseExternalDeclaration();
        if (externDecl) {
            transUnit->externDecls.push_back(externDecl);
        }
    }
    return transUnit;
}

ExternalDeclaration *Parser::parseExternalDeclaration() {
    auto begin = cursor;
    auto declSpecs = parseDeclarationSpecifiers();
    if (isKind(cursor, TK_SEMI)) {
        ++cursor;
        auto decl = new Declaration(declSpecs);
        return new ExternalDeclaration(decl);
    }
    auto declarator = parseDeclarator();
    // TODO
    return nullptr;
}

DeclarationSpecifiers *Parser::parseDeclarationSpecifiers() {
    DeclarationSpecifiers *declSpecs = new DeclarationSpecifiers;
    while (true) {
        switch ((*cursor)->getKind()) {
        case TK_TYPEDEF:
            declSpecs->addStorageClassSpecifier(new StorageClassSpecifier(StorageClassSpecifier::TYPEDEF));
            ++cursor;
            break;
        case TK_EXTERN:
            declSpecs->addStorageClassSpecifier(new StorageClassSpecifier(StorageClassSpecifier::EXTERN));
            ++cursor;
            break;
        case TK_STATIC:
            declSpecs->addStorageClassSpecifier(new StorageClassSpecifier(StorageClassSpecifier::STATIC));
            ++cursor;
            break;
        case TK_AUTO:
            declSpecs->addStorageClassSpecifier(new StorageClassSpecifier(StorageClassSpecifier::AUTO));
            ++cursor;
            break;
        case TK_REGISTER:
            declSpecs->addStorageClassSpecifier(new StorageClassSpecifier(StorageClassSpecifier::REGISTER));
            ++cursor;
            break;
        case TK_CONST:
            declSpecs->addTypeQualifier(new TypeQualifier(TypeQualifier::CONST));
            ++cursor;
            break;
        case TK_RESTRICT:
            declSpecs->addTypeQualifier(new TypeQualifier(TypeQualifier::RESTRICT));
            ++cursor;
            break;
        case TK_VOLATILE:
            declSpecs->addTypeQualifier(new TypeQualifier(TypeQualifier::VOLATILE));
            ++cursor;
            break;
        case TK_INLINE:
            declSpecs->addFuncSpecifier(new FunctionSpecifier(FunctionSpecifier::INLINE));
            ++cursor;
            break;
        case TK_VOID:
            declSpecs->addTypeSpecifier(new TypeSpecifier(TypeSpecifier::VOID));
            ++cursor;
            break;
        case TK_CHAR:
            declSpecs->addTypeSpecifier(new TypeSpecifier(TypeSpecifier::CHAR));
            ++cursor;
            break;
        case TK_SHORT:
            declSpecs->addTypeSpecifier(new TypeSpecifier(TypeSpecifier::SHORT));
            ++cursor;
            break;
        case TK_INT:
            declSpecs->addTypeSpecifier(new TypeSpecifier(TypeSpecifier::INT));
            ++cursor;
            break;
        case TK_LONG:
            declSpecs->addTypeSpecifier(new TypeSpecifier(TypeSpecifier::LONG));
            ++cursor;
            break;
        case TK_FLOAT:
            declSpecs->addTypeSpecifier(new TypeSpecifier(TypeSpecifier::FLOAT));
            ++cursor;
            break;
        case TK_DOUBLE:
            declSpecs->addTypeSpecifier(new TypeSpecifier(TypeSpecifier::DOUBLE));
            ++cursor;
            break;
        case TK_SIGNED:
            declSpecs->addTypeSpecifier(new TypeSpecifier(TypeSpecifier::SIGNED));
            ++cursor;
            break;
        case TK_UNSIGNED:
            declSpecs->addTypeSpecifier(new TypeSpecifier(TypeSpecifier::UNSIGNED));
            ++cursor;
            break;
        case TK_STRUCT:
        case TK_UNION: {
            auto structUnionSpec = parseStructOrUnionSpecifier();
            if (structUnionSpec) {
                declSpecs->addTypeSpecifier(new TypeSpecifier(structUnionSpec));
            }
        } break;
        case TK_ENUM: {
            auto enumSpec = parseEnumSpecifier();
            if (enumSpec) {
                declSpecs->addTypeSpecifier(new TypeSpecifier(enumSpec));
            }
        } break;
        case TK_IDENTIFIER:
            declSpecs->addTypeSpecifier(new TypeSpecifier((*cursor)->getStr()));
            ++cursor;
            break;
        default:
            return declSpecs;
        }
    }
}

Declarator *Parser::parseDeclarator() {
    return nullptr;
}

StructOrUnionSpecifier *Parser::parseStructOrUnionSpecifier() {
    auto begin = cursor;
    StructOrUnionSpecifier *structUnionSpec = new StructOrUnionSpecifier;
    structUnionSpec->isUnion = isKind(cursor, TK_UNION);
    ++cursor; // Consume STRUCT or UNION.
    switch ((*cursor)->getKind()) {
    case TK_IDENTIFIER:
        structUnionSpec->ident = (*cursor)->getStr();
        if (!isKind(cursor, TK_LBRACE)) {
            return structUnionSpec;
        }
    case TK_LBRACE:
        ++cursor; // Consume the '{'.
        while (isInBitSet(firstOfSpecifierQualifier, cursor) || isKind(cursor, TK_SEMI)) {
            // struct_declaration or just ';'
            auto structDecl = parseStructDeclaration();
            if (structDecl) {
                structUnionSpec->structDeclList.push_back(structDecl);
            }
        }
        assert(isKind(cursor, TK_RBRACE));
        ++cursor; // Consume the '}'.
        return structUnionSpec;
    default:
        error((*cursor)->getLoc(), "parse struct_or_union_specifier error");
        return nullptr;
    }
}

EnumSpecifier *Parser::parseEnumSpecifier() {
    return nullptr;
}

StructDeclaration *Parser::parseStructDeclaration() {
    auto begin = cursor;

    // To support struct like "struct id { ; };" which contains empty declaration.
    if (isKind(cursor, TK_SEMI)) {
        ++cursor; // Consume the ';'.
        return nullptr;
    }

    StructDeclaration *structDecl = new StructDeclaration;
    // Parse specifier_qualifier_list
    while (isInBitSet(firstOfSpecifierQualifier, cursor)) {
        auto specQual = parseSpecifierQualifier();
        if (specQual) {
            structDecl->specQualList.push_back(specQual);
        }
    }
    // Parse struct_declarator_list
    while (isInBitSet(firstOfDeclarator, cursor) || isKind(cursor, TK_COLON)) {
        auto structDeclarator = parseStructDeclarator();
        if (structDeclarator) {
            structDecl->structDeclarators.push_back(structDeclarator);
        }
        if (isKind(cursor, TK_COMMA)) {
            ++cursor; // Consume the ','.
        } else {
            break;
        }
    }
    assert(isKind(cursor, TK_SEMI));
    ++cursor; // Consume the ';'.
    return structDecl;
}

SpecifierQualifier *Parser::parseSpecifierQualifier() {
    auto begin = cursor;
    SpecifierQualifier *specQual = new SpecifierQualifier;
    switch ((*cursor)->getKind()) {
    case TK_CONST:
        specQual->var = new TypeQualifier(TypeQualifier::CONST);
        ++cursor;
        break;
    case TK_RESTRICT:
        specQual->var = new TypeQualifier(TypeQualifier::RESTRICT);
        ++cursor;
        break;
    case TK_VOLATILE:
        specQual->var = new TypeQualifier(TypeQualifier::VOLATILE);
        ++cursor;
        break;
    case TK_VOID:
        specQual->var = new TypeSpecifier(TypeSpecifier::VOID);
        ++cursor;
        break;
    case TK_CHAR:
        specQual->var = new TypeSpecifier(TypeSpecifier::CHAR);
        ++cursor;
        break;
    case TK_SHORT:
        specQual->var = new TypeSpecifier(TypeSpecifier::SHORT);
        ++cursor;
        break;
    case TK_INT:
        specQual->var = new TypeSpecifier(TypeSpecifier::INT);
        ++cursor;
        break;
    case TK_LONG:
        specQual->var = new TypeSpecifier(TypeSpecifier::LONG);
        ++cursor;
        break;
    case TK_FLOAT:
        specQual->var = new TypeSpecifier(TypeSpecifier::FLOAT);
        ++cursor;
        break;
    case TK_DOUBLE:
        specQual->var = new TypeSpecifier(TypeSpecifier::DOUBLE);
        ++cursor;
        break;
    case TK_SIGNED:
        specQual->var = new TypeSpecifier(TypeSpecifier::SIGNED);
        ++cursor;
        break;
    case TK_UNSIGNED:
        specQual->var = new TypeSpecifier(TypeSpecifier::UNSIGNED);
        ++cursor;
        break;
    case TK_STRUCT:
    case TK_UNION: {
        auto structUnionSpec = parseStructOrUnionSpecifier();
        if (structUnionSpec) {
            specQual->var = new TypeSpecifier(structUnionSpec);
        }
    } break;
    case TK_ENUM: {
        auto enumSpec = parseEnumSpecifier();
        if (enumSpec) {
            specQual->var = new TypeSpecifier(enumSpec);
        }
    } break;
    case TK_IDENTIFIER:
        specQual->var = new TypeSpecifier((*cursor)->getStr());
        ++cursor;
        break;
    default:
        return nullptr;
    }
    return specQual;
}

StructDeclarator *Parser::parseStructDeclarator() {
    auto begin = cursor;
    StructDeclarator *structDeclarator = new StructDeclarator;
    if (isInBitSet(firstOfDeclarator, cursor)) {
        auto declarator = parseDeclarator();
        if (declarator) {
            structDeclarator->declarator = declarator;
        }
    }
    if (isKind(cursor, TK_COLON)) {
        ++cursor; // Consume the ':'.
        auto condExpr = parseConditionalExpr();
        if (condExpr) {
            structDeclarator->constExpr = new ConstantExpression(condExpr);
        }
    }
    return structDeclarator;
}

ConditionalExpression *Parser::parseConditionalExpr() {
    auto begin = cursor;
    auto logicalOrExpr = parseLogicalOrExpr();
    ConditionalExpression *condExpr = new ConditionalExpression;
    if (logicalOrExpr) {
        condExpr->logiOrExpr = logicalOrExpr;
    }
    if (isKind(cursor, TK_QUESTION)) {
        ++cursor; // Consume the '?'.
        auto expr = parseExpression();
        assert(isKind(cursor, TK_COLON));
        ++cursor; // Consume the ':'.
        auto cond = parseConditionalExpr();
        if (expr && cond) {
            condExpr->expr = expr;
            condExpr->condExpr = cond;
        }
    }
    return condExpr;
}

LogicalOrExpression *Parser::parseLogicalOrExpr() {
    auto begin = cursor;
    LogicalOrExpression *logicalOrExpr = new LogicalOrExpression;
    auto logiAndExpr = parseLogicalAndExpr();
    if (logiAndExpr) {
        logicalOrExpr->exprs.push_back(logiAndExpr);
    }
    while (isKind(cursor, TK_PIPEPIPE)) {
        ++cursor;
        logiAndExpr = parseLogicalAndExpr();
        if (logiAndExpr) {
            logicalOrExpr->exprs.push_back(logiAndExpr);
        }
    }
    return logicalOrExpr;
}

LogicalAndExpression *Parser::parseLogicalAndExpr() {
    auto begin = cursor;
    LogicalAndExpression *logicalAndExpr = new LogicalAndExpression;
    auto bitOrExpr = parseBitOrExpr();
    if (bitOrExpr) {
        logicalAndExpr->exprs.push_back(bitOrExpr);
    }
    while (isKind(cursor, TK_AMPAMP)) {
        ++cursor;
        bitOrExpr = parseBitOrExpr();
        if (bitOrExpr) {
            logicalAndExpr->exprs.push_back(bitOrExpr);
        }
    }
    return logicalAndExpr;
}

BitOrExpression *Parser::parseBitOrExpr() {
    auto begin = cursor;
    BitOrExpression *bitOrExpr = new BitOrExpression;
    auto bitXorExpr = parseBitXorExpr();
    if (bitXorExpr) {
        bitOrExpr->exprs.push_back(bitXorExpr);
    }
    while (isKind(cursor, TK_PIPE)) {
        ++cursor;
        bitXorExpr = parseBitXorExpr();
        if (bitXorExpr) {
            bitOrExpr->exprs.push_back(bitXorExpr);
        }
    }
    return bitOrExpr;
}

BitXorExpression *Parser::parseBitXorExpr() {
    auto begin = cursor;
    BitXorExpression *bitXorExpr = new BitXorExpression;
    auto bitAndExpr = parseBitAndExpr();
    if (bitAndExpr) {
        bitXorExpr->exprs.push_back(bitAndExpr);
    }
    while (isKind(cursor, TK_CARET)) {
        ++cursor;
        bitAndExpr = parseBitAndExpr();
        if (bitAndExpr) {
            bitXorExpr->exprs.push_back(bitAndExpr);
        }
    }
    return bitXorExpr;
}

BitAndExpression *Parser::parseBitAndExpr() {
    auto begin = cursor;
    BitAndExpression *bitAndExpr = new BitAndExpression;
    auto equalExpr = parseEqualityExpr();
    if (equalExpr) {
        bitAndExpr->exprs.push_back(equalExpr);
    }
    while (isKind(cursor, TK_AMP)) {
        ++cursor;
        equalExpr = parseEqualityExpr();
        if (equalExpr) {
            bitAndExpr->exprs.push_back(equalExpr);
        }
    }
    return bitAndExpr;
}

EqualityExpression *Parser::parseEqualityExpr() {
    auto begin = cursor;
    EqualityExpression *equalityExpr = new EqualityExpression;
    auto relatExpr = parseRelationalExpr();
    if (relatExpr) {
        equalityExpr->relatExpr = relatExpr;
    }
    while (isKind(cursor, TK_EQUALEQUAL) || isKind(cursor, TK_EXCLAIMEQUAL)) {
        TokenKind kind = (*cursor)->getKind();
        auto op = (kind == TK_EQUALEQUAL) ? EqualityExpression::EQUALEQUAL : EqualityExpression::EXCLAIMEQUAL;
        ++cursor;
        relatExpr = parseRelationalExpr();
        if (relatExpr) {
            equalityExpr->exprs.emplace_back(op, relatExpr);
        }
    }
    return equalityExpr;
}

RelationalExpression *Parser::parseRelationalExpr() {
    auto begin = cursor;
    RelationalExpression *relatExpr = new RelationalExpression;
    auto shiftExpr = parseShiftExpr();
    if (shiftExpr) {
        relatExpr->shiftExpr = shiftExpr;
    }
    auto kind = (*cursor)->getKind();
    while (kind == TK_LESS || kind == TK_GREATER || kind == TK_LESSEQUAL || kind == TK_GREATEREQUAL) {
        RelationalExpression::REOp op;
        switch (kind) {
        case TK_LESS:
            op = RelationalExpression::LESS;
            break;
        case TK_GREATER:
            op = RelationalExpression::GREATER;
            break;
        case TK_LESSEQUAL:
            op = RelationalExpression::LESSEQUAL;
            break;
        case TK_GREATEREQUAL:
            op = RelationalExpression::GREATEREQUAL;
            break;
        }
        ++cursor;
        shiftExpr = parseShiftExpr();
        if (shiftExpr) {
            relatExpr->exprs.emplace_back(op, shiftExpr);
        }
        kind = (*cursor)->getKind();
    }
    return relatExpr;
}

ShiftExpression *Parser::parseShiftExpr() {
    auto begin = cursor;
    ShiftExpression *shiftExpr = new ShiftExpression;
    auto addExpr = parseAdditiveExpr();
    if (addExpr) {
        shiftExpr->addiExpr = addExpr;
    }
    while (isKind(cursor, TK_LESSLESS) || isKind(cursor, TK_GREATERGREATER)) {
        TokenKind kind = (*cursor)->getKind();
        auto op = (kind == TK_LESSLESS) ? ShiftExpression::LESSLESS : ShiftExpression::GREATERGREATER;
        ++cursor;
        addExpr = parseAdditiveExpr();
        if (addExpr) {
            shiftExpr->exprs.emplace_back(op, addExpr);
        }
    }
    return shiftExpr;
}

Expr *Parser::parseAdditiveExpression() {
    Expr *lhs = parseMultiplicativeExpression();
    while (true) {
        BinaryOpKind op;
        switch ((*cursor)->getKind()) {
        case TK_PLUS:
            op = BinaryOpKind::ADD;
            break;
        case TK_MINUS:
            op = BinaryOpKind::SUB;
            break;
        default:
            return lhs;
        }
        SourceLocation loc = (*cursor)->getLoc();
        ++cursor;
        Expr *rhs = parseMultiplicativeExpression();
        lhs = new BinaryExpr(loc, op, lhs, rhs);
    }
}

Expr *Parser::parseMultiplicativeExpression() {
    Expr *lhs = parseCastExpression();
    while (true) {
        BinaryOpKind op;
        switch ((*cursor)->getKind()) {
        case TK_STAR:
            op = BinaryOpKind::MUL;
            break;
        case TK_SLASH:
            op = BinaryOpKind::DIV;
            break;
        case TK_PERCENT:
            op = BinaryOpKind::MOD;
            break;
        default:
            return lhs;
        }
        SourceLocation loc = (*cursor)->getLoc();
        ++cursor;
        Expr *rhs = parseCastExpression();
        lhs = new BinaryExpr(loc, op, lhs, rhs);
    }
}

Expr *Parser::parseCastExpression() {
    auto begin = cursor;
    CastExpression *castExpr = new CastExpression;
    auto nxt = std::next(cursor, 1);
    if (isKind(cursor, TK_LPAR) && isFirstOfTypeName(nxt)) {
        ++cursor; // Consume the '('.
        TypeName *typeName = parseTypeName();
        assert(isKind(cursor, TK_RPAR));
        ++cursor; // Consume the ')'.
        if (isKind(cursor, TK_LBRACE)) {
            // Compound literal
            cursor = begin;
            UnaryExpression *unary = parseUnaryExpr();
            castExpr->var = unary;
        } else {
            // Type cast
            CastExpression *cast = parseCastExpr();
            castExpr->var = std::make_pair(typeName, cast);
        }
    } else {
        UnaryExpression *unary = parseUnaryExpr();
        castExpr->var = unary;
    }
    return castExpr;
}

UnaryExpression *Parser::parseUnaryExpr() {
    auto begin = cursor;
    UnaryExpression *unaryExpr = new UnaryExpression;
    if (isKind(cursor, TK_SIZEOF)) {
        ++cursor;
        auto nxt = std::next(cursor, 1);
        if (isKind(cursor, TK_LPAR) && isFirstOfTypeName(nxt)) {
            ++cursor;
            auto typeName = parseTypeName();
            assert(isKind(cursor, TK_RPAR));
        }
    }
    return nullptr;
}

Expression *Parser::parseExpression() {
    return nullptr;
}

TypeName *Parser::parseTypeName() {
    return nullptr;
}

InitializerList *Parser::parseInitializerList() {
    return nullptr;
}