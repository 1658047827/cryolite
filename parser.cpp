#include "parser.h"
#include "diagnostic.h"
#include <cassert>

unsigned DeclSpec::getParsedSpecifiers() {
    unsigned ret = 0;
    if (storageClassSpec != SCS_UNSPECIFIED)
        ret |= PQ_STORAGE_CLASS_SPECIFIER;
    if (typeQualifiers != TQ_UNSPECIFIED)
        ret |= PQ_TYPE_QUALIFIER;
    if (hasTypeSpecifier())
        ret |= PQ_TYPE_SPECIFIER;
    if (fsInlineSpecified)
        ret |= PQ_FUNCTION_SPECIFIER;
    return ret;
}

bool DeclSpec::setStorageClassSpec(SCS s, SourceLocation loc) {
    if (storageClassSpec != SCS_UNSPECIFIED) {
        error(loc, "invalid declaration specifier combination");
        return true;
    }
    storageClassSpec = s;
    storageClassSpecLoc = loc;
    return false;
}

bool DeclSpec::setTypeSpecWidth(TSW w, SourceLocation loc) {
    if (typeSpecWidth != TSW_UNSPECIFIED &&
        // Only allow turning long -> long long.
        (w != TSW_LONGLONG || typeSpecWidth != TSW_LONG)) {
        error(loc, "invalid declaration specifier combination");
        return true;
    }
    typeSpecWidth = w;
    tswLoc = loc;
    return false;
}

bool DeclSpec::setTypeSpecSign(TSS s, SourceLocation loc) {
    if (typeSpecSign != TSS_UNSPECIFIED) {
        error(loc, "invalid declaration specifier combination");
        return true;
    }
    typeSpecSign = s;
    tssLoc = loc;
    return false;
}

bool DeclSpec::setTypeSpecType(TST t, SourceLocation loc, void *rep, bool owned) {
    if (typeSpecType != TST_UNSPECIFIED) {
        error(loc, "invalid declaration specifier combination");
        return true;
    }
    typeSpecType = t;
    typeRep = rep;
    tstLoc = loc;
    typeSpecOwned = owned;
    return false;
}

bool DeclSpec::setTypeSpecError() {
    typeSpecType = TST_ERROR;
    tstLoc = SourceLocation();
    return false;
}

bool DeclSpec::setTypeQual(TQ t, SourceLocation loc) {
    if (typeQualifiers & t) {
        error(loc, "duplicate declaration specifier");
        return true;
    }
    typeQualifiers |= t;
    switch (t) {
    case TQ_CONST:
        tqConstLoc = loc;
        break;
    case TQ_RESTRICT:
        tqRestrictLoc = loc;
        break;
    case TQ_VOLATILE:
        tqVolatileLoc = loc;
        break;
    default:
        assert(0 && "Unknown type qualifier");
    }
    return false;
}

bool DeclSpec::setFunctionSpecInline(SourceLocation loc) {
    // Warning 'inline inline'.
    if (fsInlineSpecified) {
        warning(loc, "duplicate 'inline' declaration specifier");
    }
    fsInlineSpecified = true;
    fsInlineLoc = loc;
    return false;
}

void DeclSpec::finish() {
    // signed/unsigned are only valid with int/char.
    if (typeSpecSign != TSS_UNSPECIFIED) {
        if (typeSpecType == TST_UNSPECIFIED)
            typeSpecType = TST_INT; // unsigned -> unsigned int, signed -> signed int.
        else if (typeSpecType != TST_INT && typeSpecType != TST_CHAR) {
            error(tssLoc, "invalid sign specifier");
            // signed float -> float.
            typeSpecSign = TSS_UNSPECIFIED;
        }
    }

    // Validate the width of the type.
    switch (typeSpecWidth) {
    case TSW_UNSPECIFIED:
        break;
    case TSW_SHORT:    // short int
    case TSW_LONGLONG: // long long int
        if (typeSpecType == TST_UNSPECIFIED)
            typeSpecType = TST_INT; // short -> short int, long long -> long long int.
        else if (typeSpecType != TST_INT) {
            error(tswLoc, typeSpecWidth == TSW_SHORT ? "invalid short specifier"
                                                     : "invalid long long specifier");
            typeSpecType = TST_INT;
        }
        break;
    case TSW_LONG: // long double, long int
        if (typeSpecType == TST_UNSPECIFIED)
            typeSpecType = TST_INT; // long -> long int.
        else if (typeSpecType != TST_INT && typeSpecType != TST_DOUBLE) {
            error(tswLoc, "invalid long specifier");
            typeSpecType = TST_INT;
        }
        break;
    }
}

bool DeclSpec::isMissingDeclaratorOk() {
    return (typeSpecType == TST_UNION ||
            typeSpecType == TST_STRUCT ||
            typeSpecType == TST_ENUM) &&
           storageClassSpec != SCS_TYPEDEF;
}

DeclaratorChunk DeclaratorChunk::getPointer(unsigned char typeQuals, SourceLocation loc) {
    DeclaratorChunk dc;
    dc.chunkKind = POINTER;
    dc.loc = loc;
    dc.ptr.typeQuals = typeQuals;
    return dc;
}

DeclaratorChunk DeclaratorChunk::getArray(unsigned char typeQuals,
                                          bool isStatic,
                                          bool isStar,
                                          void *numElts,
                                          SourceLocation loc) {
    DeclaratorChunk dc;
    dc.chunkKind = ARRAY;
    dc.loc = loc;
    dc.arr.typeQuals = typeQuals;
    dc.arr.hasStaic = isStatic;
    dc.arr.isStar = isStar;
    dc.arr.numElts = numElts;
    return dc;
}

DeclaratorChunk DeclaratorChunk::getFunction(unsigned char typeQuals,
                                             bool hasProto,
                                             bool isVariadic,
                                             SourceLocation loc) {
    DeclaratorChunk dc;
    dc.chunkKind = FUNCTION;
    dc.loc = loc;
    dc.fun.hasPrototype = hasProto;
    dc.fun.isVariadic = isVariadic;
    dc.fun.typeQuals = typeQuals;
    return dc;
}

QualType convertDeclSpecToType(const DeclSpec &ds, SourceLocation loc, bool &isInvalid) {
    QualType result;
    switch (ds.typeSpecType) {
    case DeclSpec::TST_VOID:
        result.type = VoidType::getVoidType();
        break;
    case DeclSpec::TST_CHAR:
        if (ds.typeSpecSign == DeclSpec::TSS_UNSPECIFIED)
            result.type = BuiltinType::getBuiltinType(BuiltinType::CHAR);
        else if (ds.typeSpecSign == DeclSpec::TSS_SIGNED)
            result.type = BuiltinType::getBuiltinType(BuiltinType::SIGNED | BuiltinType::CHAR);
        else {
            assert(ds.typeSpecSign == DeclSpec::TSS_UNSIGNED && "Unknown TSS value");
            result.type = BuiltinType::getBuiltinType(BuiltinType::UNSIGNED | BuiltinType::CHAR);
        }
        break;
    case DeclSpec::TST_UNSPECIFIED:
        if (!ds.hasTypeSpecifier()) {
            error(loc, "missing type specifier");
        }
    // Fall through
    case DeclSpec::TST_INT: {
        if (ds.typeSpecSign != DeclSpec::TSS_UNSIGNED) {
            switch (ds.typeSpecWidth) {
            case DeclSpec::TSW_UNSPECIFIED:
                result.type = BuiltinType::getBuiltinType(BuiltinType::INT);
                break;
            case DeclSpec::TSW_SHORT:
                result.type = BuiltinType::getBuiltinType(BuiltinType::SHORT);
                break;
            case DeclSpec::TSW_LONG:
                result.type = BuiltinType::getBuiltinType(BuiltinType::LONG);
                break;
            case DeclSpec::TSW_LONGLONG:
                result.type = BuiltinType::getBuiltinType(BuiltinType::LONGLONG);
                break;
            }
        } else {
            switch (ds.typeSpecWidth) {
            case DeclSpec::TSW_UNSPECIFIED:
                result.type = BuiltinType::getBuiltinType(BuiltinType::UNSIGNED | BuiltinType::INT);
                break;
            case DeclSpec::TSW_SHORT:
                result.type = BuiltinType::getBuiltinType(BuiltinType::UNSIGNED | BuiltinType::SHORT);
                break;
            case DeclSpec::TSW_LONG:
                result.type = BuiltinType::getBuiltinType(BuiltinType::UNSIGNED | BuiltinType::LONG);
                break;
            case DeclSpec::TSW_LONGLONG:
                result.type = BuiltinType::getBuiltinType(BuiltinType::UNSIGNED | BuiltinType::LONGLONG);
                break;
            }
        }
        break;
    }
    case DeclSpec::TST_FLOAT:
        result.type = BuiltinType::getBuiltinType(BuiltinType::FLOAT);
        break;
    case DeclSpec::TST_DOUBLE:
        if (ds.typeSpecWidth == DeclSpec::TSW_LONG)
            result.type = BuiltinType::getBuiltinType(BuiltinType::LONG | BuiltinType::DOUBLE);
        else
            result.type = BuiltinType::getBuiltinType(BuiltinType::DOUBLE);
        break;
    case DeclSpec::TST_BOOL:
        // TODO: Support bool.
        break;
    case DeclSpec::TST_ENUM:
        // TODO
        break;
    case DeclSpec::TST_UNION:
    case DeclSpec::TST_STRUCT: {
        Decl *d = static_cast<Decl *>(ds.typeRep);
        RecordDecl *rd = dynamic_cast<RecordDecl *>(d);
        result.type = new RecordType(rd);
        break;
    }
    case DeclSpec::TST_TYPENAME: {
        // TODO
    }
    }

    // Apply const/volatile/restrict qualifiers to T.
    if (unsigned typeQuals = ds.typeQualifiers) {
    }

    return result;
}

QualType getTypeForDeclarator(Declarator &d) {
    QualType t;
    switch (d.kind) {
    case Declarator::DK_ABSTRACT:
    case Declarator::DK_NORMAL: {
        const DeclSpec &ds = d.ds;
        bool isInvalid = false;
        t = convertDeclSpecToType(ds, d.identifierLoc, isInvalid);
        if (isInvalid)
            d.invalidType = true;
        break;
    }
    }

    // TODO
    for (unsigned i = 0, e = d.declTypeInfo.size(); i != e; ++i) {
        DeclaratorChunk &declType = d.declTypeInfo[e - i - 1];
        switch (declType.chunkKind) {
        case DeclaratorChunk::POINTER:
            break;
        case DeclaratorChunk::ARRAY:
            break;
        case DeclaratorChunk::FUNCTION:
            break;
        default:
            break;
        }
    }

    return t;
}

unsigned int parseIntegerSuffix(NumericLiteralParser &parser) {
    // Integer constants are implicitly of type int by default.
    unsigned int ret = BuiltinType::INT;
    if (parser.isLong) {
        ret |= BuiltinType::LONG;
    } else if (parser.isLongLong) {
        ret |= BuiltinType::LONGLONG;
    }
    if (parser.isUnsigned) {
        ret |= BuiltinType::UNSIGNED;
    }
    return ret;
}

unsigned int parseFloatingSuffix(NumericLiteralParser &parser) {
    // Floating constants are implicitly of type double by default.
    unsigned int ret = BuiltinType::DOUBLE;
    if (parser.isFloat) {
        ret |= BuiltinType::FLOAT;
        ret &= ~BuiltinType::DOUBLE;
    } else if (parser.isLong) {
        ret |= BuiltinType::LONG;
    }
    return ret;
}

inline bool Parser::isKind(TokenSeqConstIter &iter, TokenKind kind) {
    return (*iter)->getKind() == kind;
}

void Parser::expect(TokenSeqConstIter &iter, TokenKind kind) {
    if ((*iter)->getKind() != kind) {
        error((*iter)->getLoc(), "expected '" + tokenKind2Str.at(kind) + "' here");
    }
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
    return nullptr;
}

Expr *Parser::parseExpression() {
    return parseSimpleBinaryExpression(&Parser::parseAssignmentExpression, TK_COMMA, BinaryOpKind::COMMA);
}

/**
 * According to C99:
 *
 * assignment-expression:
 *     conditional-expression
 *     unary-expression assignment-operator assignment-expression
 *
 * assignment-operator: one of
 *     =
 *     *= /= %=
 *     += -=
 *     <<= >>=
 *     &= ^= |=
 *
 * Instead we are doing something similar to clang here.
 * We use the grammar of the form:
 *
 * assignment-expression:
 *     conditional-expression
 *     conditional-expression assignment-operator assignment-expression
 *
 * Checking if the lhs is an LValue will be done by type checking inside BinaryExpr.
 */
Expr *Parser::parseAssignmentExpression() {
    // Let the BinaryExpr handle the unexpected expression kind.
    Expr *lhs = parseConditionalExpression();
    // Because this is a right-associative expression, we cannot use a loop structure
    // in the same manner as we do with left-associative expressions.
    BinaryOpKind op;
    switch ((*cursor)->getKind()) {
    case TK_EQUAL:
        op = BinaryOpKind::ASSIGN;
        break;
    case TK_STAREQUAL:
        op = BinaryOpKind::MUL;
        break;
    case TK_SLASHEQUAL:
        op = BinaryOpKind::DIV;
        break;
    case TK_PERCENTEQUAL:
        op = BinaryOpKind::MOD;
        break;
    case TK_PLUSEQUAL:
        op = BinaryOpKind::ADD;
        break;
    case TK_MINUSEQUAL:
        op = BinaryOpKind::SUB;
        break;
    case TK_LESSLESSEQUAL:
        op = BinaryOpKind::SHL;
        break;
    case TK_GREATERGREATEREQUAL:
        op = BinaryOpKind::SHR;
        break;
    case TK_AMPEQUAL:
        op = BinaryOpKind::BITAND;
        break;
    case TK_CARETEQUAL:
        op = BinaryOpKind::BITXOR;
        break;
    case TK_PIPEEQUAL:
        op = BinaryOpKind::BITOR;
        break;
    default:
        // Now it seems to be a single conditional expression.
        return lhs;
    }
    SourceLocation loc = (*cursor)->getLoc();
    ++cursor;
    Expr *rhs = parseAssignmentExpression();
    // Break compound assignment into two BinaryExprs.
    // e.g.: a += b => a = a + b
    if (op != BinaryOpKind::ASSIGN) {
        rhs = new BinaryExpr(loc, op, lhs, rhs);
    }
    return new BinaryExpr(loc, BinaryOpKind::ASSIGN, lhs, rhs);
}

Expr *Parser::parseConditionalExpression() {
    Expr *condExpr = parseLogicalOrExpression();
    if (isKind(cursor, TK_QUESTION)) {
        SourceLocation loc = (*cursor)->getLoc();
        ++cursor;
        Expr *trueExpr = parseExpression();
        expect(cursor, TK_COLON);
        ++cursor;
        Expr *falseExpr = parseConditionalExpression();
        return new TernaryExpr(loc, condExpr, trueExpr, falseExpr);
    }
    return condExpr;
}

Expr *Parser::parseSimpleBinaryExpression(Expr *(Parser::*parseTerm)(), TokenKind kind, BinaryOpKind op) {
    Expr *lhs = (this->*parseTerm)();
    while (isKind(cursor, kind)) {
        SourceLocation loc = (*cursor)->getLoc();
        ++cursor;
        Expr *rhs = (this->*parseTerm)();
        lhs = new BinaryExpr(loc, op, lhs, rhs);
    }
    return lhs;
}

Expr *Parser::parseLogicalOrExpression() {
    return parseSimpleBinaryExpression(&Parser::parseLogicalAndExpression, TK_PIPEPIPE, BinaryOpKind::LOGICOR);
}

Expr *Parser::parseLogicalAndExpression() {
    return parseSimpleBinaryExpression(&Parser::parseBitOrExpression, TK_AMPAMP, BinaryOpKind::LOGICAND);
}

Expr *Parser::parseBitOrExpression() {
    return parseSimpleBinaryExpression(&Parser::parseBitXorExpression, TK_PIPE, BinaryOpKind::BITOR);
}

Expr *Parser::parseBitXorExpression() {
    return parseSimpleBinaryExpression(&Parser::parseBitAndExpression, TK_CARET, BinaryOpKind::BITXOR);
}

Expr *Parser::parseBitAndExpression() {
    return parseSimpleBinaryExpression(&Parser::parseEqualityExpression, TK_AMP, BinaryOpKind::BITAND);
}

Expr *Parser::parseEqualityExpression() {
    Expr *lhs = parseRelationalExpression();
    while (true) {
        BinaryOpKind op;
        switch ((*cursor)->getKind()) {
        case TK_EQUALEQUAL:
            op = BinaryOpKind::EQUAL;
            break;
        case TK_EXCLAIMEQUAL:
            op = BinaryOpKind::NEQ;
            break;
        default:
            return lhs;
        }
        SourceLocation loc = (*cursor)->getLoc();
        ++cursor;
        Expr *rhs = parseRelationalExpression();
        lhs = new BinaryExpr(loc, op, lhs, rhs);
    }
}

Expr *Parser::parseRelationalExpression() {
    Expr *lhs = parseShiftExpression();
    while (true) {
        BinaryOpKind op;
        switch ((*cursor)->getKind()) {
        case TK_LESS:
            op = BinaryOpKind::LESS;
            break;
        case TK_LESSEQUAL:
            op = BinaryOpKind::LEQ;
            break;
        case TK_GREATER:
            op = BinaryOpKind::GREATER;
            break;
        case TK_GREATEREQUAL:
            op = BinaryOpKind::GEQ;
            break;
        default:
            return lhs;
        }
        SourceLocation loc = (*cursor)->getLoc();
        ++cursor;
        Expr *rhs = parseShiftExpression();
        lhs = new BinaryExpr(loc, op, lhs, rhs);
    }
}

Expr *Parser::parseShiftExpression() {
    Expr *lhs = parseAdditiveExpression();
    while (true) {
        BinaryOpKind op;
        switch ((*cursor)->getKind()) {
        case TK_LESSLESS:
            op = BinaryOpKind::SHL;
            break;
        case TK_GREATERGREATER:
            op = BinaryOpKind::SHR;
            break;
        default:
            return lhs;
        }
        SourceLocation loc = (*cursor)->getLoc();
        ++cursor;
        Expr *rhs = parseAdditiveExpression();
        lhs = new BinaryExpr(loc, op, lhs, rhs);
    }
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
    TokenSeqConstIter nxt = std::next(cursor, 1);
    if (isKind(cursor, TK_LPAR) && isFirstOfTypeName(nxt)) {
        // TODO
        return nullptr;
    } else {
        return parseUnaryExpression();
    }
}

Expr *Parser::parseUnaryExpression() {
    switch ((*cursor)->getKind()) {
    case TK_PLUSPLUS:
        return parsePrefixIncrementOrDecrement(UnaryOpKind::PREINC);
    case TK_MINUSMINUS:
        return parsePrefixIncrementOrDecrement(UnaryOpKind::PREDEC);
    case TK_AMP:
        return parseUnaryOperatorExpression(UnaryOpKind::ADDR);
    case TK_STAR:
        return parseUnaryOperatorExpression(UnaryOpKind::DEREF);
    case TK_PLUS:
        return parseUnaryOperatorExpression(UnaryOpKind::PLUS);
    case TK_MINUS:
        return parseUnaryOperatorExpression(UnaryOpKind::MINUS);
    case TK_TILDE:
        return parseUnaryOperatorExpression(UnaryOpKind::BITNOT);
    case TK_EXCLAIM:
        return parseUnaryOperatorExpression(UnaryOpKind::LOGICNOT);
    case TK_SIZEOF:
        return parseSizeof();
    default:
        return parsePostfixExpression();
    }
}

UnaryExpr *Parser::parsePrefixIncrementOrDecrement(UnaryOpKind op) {
    SourceLocation loc = (*cursor)->getLoc();
    ++cursor;
    Expr *operand = parseUnaryExpression();
    return new UnaryExpr(loc, op, operand);
}

UnaryExpr *Parser::parseUnaryOperatorExpression(UnaryOpKind op) {
    SourceLocation loc = (*cursor)->getLoc();
    ++cursor; // Consume the unary operator.
    Expr *expr = parseCastExpression();
    return new UnaryExpr(loc, op, expr);
}

SizeofExpr *Parser::parseSizeof() {
    SourceLocation loc = (*cursor)->getLoc();
    ++cursor; // Consume the SIZEOF.
    TokenSeqConstIter nxt = std::next(cursor, 1);
    if (isKind(cursor, TK_LPAR) && isFirstOfTypeName(nxt)) {
        ++cursor; // Consume the '('.
        QualType qt = parseTypeName();
        expect(cursor, TK_RPAR);
        ++cursor; // Consume the ')'.
        return new SizeofExpr(loc, qt);
    } else {
        Expr *expr = parseUnaryExpression();
        return new SizeofExpr(loc, expr);
    }
}

Expr *Parser::parsePostfixExpression() {
    // Postfix expression, which always starts with a compound literal or primary expression,
    // is followed by a series of suffixes like '++', '--', '->', '.' etc.
    Expr *expr;
    if (isKind(cursor, TK_LPAR) && isFirstOfTypeName(cursor)) {
        expr = parseCompoundLiteral();
    } else {
        expr = parsePrimaryExpression();
    }
    return parsePostfixExpressionSuffix(expr);
}

Expr *Parser::parsePostfixExpressionSuffix(Expr *expr) {
    while (true) {
        switch ((*cursor)->getKind()) {
        case TK_LSQB:
        case TK_LPAR:
        case TK_DOT:
        case TK_ARROW:
        case TK_PLUSPLUS:
            expr = new UnaryExpr((*cursor)->getLoc(), POSINC, expr);
            break;
        case TK_MINUSMINUS:
            expr = new UnaryExpr((*cursor)->getLoc(), POSDEC, expr);
            break;
        default:
            return expr;
        }
        ++cursor;
    }
}

Expr *Parser::parseCompoundLiteral() {
    // TODO: Complete this function.
    return nullptr;
}

Expr *Parser::parseArraySubscripting() {
    // TODO: Complete this function.
    return nullptr;
}

Expr *Parser::parsePrimaryExpression() {
    Expr *ret;
    SourceLocation loc = (*cursor)->getLoc();
    switch ((*cursor)->getKind()) {
    case TK_IDENTIFIER:
        return parseIdentifier();
    case TK_NUMERIC_CONSTANT: {
        std::string tok = (*cursor)->getStr();
        NumericLiteralParser parser(tok.data(), tok.data() + tok.size(), loc);
        if (parser.hadError) {
            // TODO: A better way?
            return nullptr;
        } else if (parser.isIntegerLiteral()) {
            return parseIntegerConstant(parser);
        } else if (parser.isFloatingLiteral()) {
            return parseFloatingConstant(parser);
        }
    }
    case TK_CHAR_CONSTANT:
        return parseCharacterConstant();
    case TK_STRING_LITERAL:
        return parseStringLiterals();
    case TK_LPAR:
        return parseParenthesesExpression();
    default:
        error(loc, "expected primary expression");
        return nullptr;
    }
}

DeclRefExpr *Parser::parseIdentifier() {
}

Expr *Parser::parseParenthesesExpression() {
    expect(cursor, TK_LPAR);
    ++cursor;
    Expr *expr = parseExpression();
    expect(cursor, TK_RPAR);
    // Maintain consistency: after parseXXX, cursor always points to next token.
    ++cursor;
    return expr;
}

IntegerConstant *Parser::parseIntegerConstant(NumericLiteralParser &parser) {
    SourceLocation loc = (*cursor)->getLoc();
    std::string tok = (*cursor)->getStr();
    unsigned long long val = 0;
    size_t idx = 0;
    try {
        val = std::stoull(tok, &idx, parser.getRadix());
    } catch (const std::out_of_range &e) {
        error(loc, "integer constant too large");
        val = 0;
    }
    ++cursor; // Maintain consistency.
    // TODO: Automatically determine the type based on the size of the integer constant.
    unsigned int builtinTypeFlags = parseIntegerSuffix(parser);
    BuiltinType *builtinType = BuiltinType::getBuiltinType(builtinTypeFlags);
    return new IntegerConstant(loc, QualType(builtinType), val);
}

FloatingConstant *Parser::parseFloatingConstant(NumericLiteralParser &parser) {
    SourceLocation loc = (*cursor)->getLoc();
    std::string tok = (*cursor)->getStr();
    long double val;
    size_t idx = 0;
    try {
        val = std::stold(tok, &idx);
    } catch (const std::out_of_range &e) {
        error(loc, "floating constant too large");
        val = 0.0;
    }
    ++cursor; // Maintain consistency.
    // TODO: Automatically determine the type based on the size of the floating constant.
    unsigned int builtinTypeFlags = parseFloatingSuffix(parser);
    BuiltinType *builtinType = BuiltinType::getBuiltinType(builtinTypeFlags);
    return new FloatingConstant(loc, QualType(builtinType), val);
}

CharacterConstant *Parser::parseCharacterConstant() {
    SourceLocation loc = (*cursor)->getLoc();
    std::string tok = (*cursor)->getStr(); // "'c'"
    unsigned int val;
    // TODO: Support more kinds of character constants.
    val = tok[1];
    ++cursor; // Maintain consistency.
    BuiltinType *builtinType = BuiltinType::getBuiltinType(BuiltinType::INT);
    return new CharacterConstant(loc, QualType(builtinType), val);
}

StringLiteral *Parser::parseStringLiterals() {
    // TODO: Support more kinds of string literals.
    SourceLocation loc = (*cursor)->getLoc();
    std::string val;
    // Continuous string literals like "hello" " world" produces
    // a single string literal "hello world".
    while (isKind(cursor, TK_STRING_LITERAL)) {
        std::string tmp = (*cursor)->getStr();
        // Remove "".
        tmp.erase(0, 1);
        tmp.pop_back();
        val.append(tmp);
        ++cursor;
    }
    size_t sz = val.size() + 1; // '\0'
    BuiltinType *charT = BuiltinType::getBuiltinType(BuiltinType::CHAR);
    ConstantArrayType *constArrT = new ConstantArrayType(charT, sz);
    return new StringLiteral(loc, QualType(constArrT), val);
}

/**
 * declaration-specifiers:
 *     storage-class-specifier declaration-specifiers{opt}
 *     type-specifier declaration-specifiers{opt}
 *     type-qualifier declaration-specifiers{opt}
 *     function-specifier declaration-specifiers{opt}
 */
void Parser::parseDeclarationSpecifiers(DeclSpec &ds) {
    while (true) {
        bool isInvalid = false;
        SourceLocation loc = (*cursor)->getLoc();

        switch ((*cursor)->getKind()) {
        default:
        DoneWithDeclSpec:
            ds.finish();
            return;

        // typdef-name
        case TK_IDENTIFIER: {
            // This identifier can only be a typedef name if we haven't already seen
            // a type-specifier. Without this check we misparse:
            // typedef int X; struct Y { short X; }; as 'short int'.
            if (ds.hasTypeSpecifier())
                goto DoneWithDeclSpec;

            // TODO: getTypeName like Clang, and DeclSpec needs a void *typeRep.

            isInvalid = ds.setTypeSpecType(DeclSpec::TST_TYPENAME, loc);
            if (isInvalid) break;

            ++cursor; // Consume the identifier.

            continue;
        }

        // storage-class-specifier
        case TK_TYPEDEF:
            isInvalid = ds.setStorageClassSpec(DeclSpec::SCS_TYPEDEF, loc);
            break;
        case TK_EXTERN:
            isInvalid = ds.setStorageClassSpec(DeclSpec::SCS_EXTERN, loc);
            break;
        case TK_STATIC:
            isInvalid = ds.setStorageClassSpec(DeclSpec::SCS_STATIC, loc);
            break;
        case TK_AUTO:
            isInvalid = ds.setStorageClassSpec(DeclSpec::SCS_AUTO, loc);
            break;
        case TK_REGISTER:
            isInvalid = ds.setStorageClassSpec(DeclSpec::SCS_REGISTER, loc);
            break;

        // function-specifier
        case TK_INLINE:
            isInvalid = ds.setFunctionSpecInline(loc);
            break;

        // type-specifier
        case TK_SHORT:
            isInvalid = ds.setTypeSpecWidth(DeclSpec::TSW_SHORT, loc);
            break;
        case TK_LONG:
            if (ds.typeSpecWidth != DeclSpec::TSW_LONG)
                isInvalid = ds.setTypeSpecWidth(DeclSpec::TSW_LONG, loc);
            else
                isInvalid = ds.setTypeSpecWidth(DeclSpec::TSW_LONGLONG, loc);
            break;
        case TK_SIGNED:
            isInvalid = ds.setTypeSpecSign(DeclSpec::TSS_SIGNED, loc);
            break;
        case TK_UNSIGNED:
            isInvalid = ds.setTypeSpecSign(DeclSpec::TSS_UNSIGNED, loc);
            break;
        case TK_VOID:
            isInvalid = ds.setTypeSpecType(DeclSpec::TST_VOID, loc);
            break;
        case TK_CHAR:
            isInvalid = ds.setTypeSpecType(DeclSpec::TST_CHAR, loc);
            break;
        case TK_INT:
            isInvalid = ds.setTypeSpecType(DeclSpec::TST_INT, loc);
            break;
        case TK_FLOAT:
            isInvalid = ds.setTypeSpecType(DeclSpec::TST_FLOAT, loc);
            break;
        case TK_DOUBLE:
            isInvalid = ds.setTypeSpecType(DeclSpec::TST_DOUBLE, loc);
            break;
        // TODO: Supprt bool.

        // record-specifier
        case TK_STRUCT:
        case TK_UNION: {
            parseRecordSpecifier(loc, ds);
            continue;
        }

        // enum-specifier
        case TK_ENUM:
            ++cursor;
            parseEnumSpecifier(loc, ds);
            continue;

        // type-qualifier
        case TK_CONST:
            isInvalid = ds.setTypeQual(DeclSpec::TQ_CONST, loc);
            break;
        case TK_RESTRICT:
            isInvalid = ds.setTypeQual(DeclSpec::TQ_RESTRICT, loc);
            break;
        case TK_VOLATILE:
            isInvalid = ds.setTypeQual(DeclSpec::TQ_VOLATILE, loc);
            break;
        }

        // If the specifier wasn't legal, issue a diagnostic.
        if (isInvalid) {
            error(loc, "illegal declaration specifiers");
        }
        ++cursor;
    }
}

void Parser::parseSpecifierQualifierList(DeclSpec &ds) {
    // specifier-qualifier-list is a subset of declaration-specifiers.
    // Just parse declaration-specifiers and complain about extra stuff.
    parseDeclarationSpecifiers(ds);

    // TODO
}

QualType Parser::parseTypeName() {
    // Parse the common declaration-specifiers piece.
    DeclSpec ds;
    parseSpecifierQualifierList(ds);

    // Parse the abstract-declarator, if present.
    Declarator declaratorInfo(ds, Declarator::TYPE_NAME_CONTEXT);
    parseDeclarator(declaratorInfo);

    if (declaratorInfo.isInvalidType())
        return QualType(nullptr);
}

void Parser::parseEnumSpecifier(SourceLocation loc, DeclSpec &ds) {
}

void Parser::parseRecordSpecifier(SourceLocation loc, DeclSpec &ds) {
    DeclSpec::TST tagType;
    TokenKind k = (*cursor)->getKind();
    ++cursor; // Consume struct-or-union.
    if (k == TK_STRUCT)
        tagType = DeclSpec::TST_STRUCT;
    else if (k == TK_UNION)
        tagType = DeclSpec::TST_UNION;
    else
        assert(0 && "Not a record specifier");

    // Parse the (optional) record name.
    std::string name;
    SourceLocation nameLoc;
    if (isKind(cursor, TK_IDENTIFIER)) {
        name = (*cursor)->getStr();
        nameLoc = (*cursor)->getLoc();
        ++cursor;
    }

    // There are three options here.
    // If we have 'struct foo;', then this is a forward declaration.
    // If we have 'struct foo {...' then this is a definition.
    // Otherwise we have something like 'struct foo x', a reference.
    TagUseKind tagUseKind;
    if (isKind(cursor, TK_LBRACE))
        tagUseKind = DEFINITION;
    else if (isKind(cursor, TK_SEMI))
        tagUseKind = DECLARATION;
    else
        tagUseKind = REFERENCE;

    if (name.empty() && tagUseKind != DEFINITION) {
        error(loc, "illegal anonymous record");
        // Skip the rest of this declarator, up until the comma or semicolon.
        while (!isKind(cursor, TK_COMMA) && !isKind(cursor, TK_SEMI))
            ++cursor;
        return;
    }

    bool Owned = false;
    RecordDecl *record = parseStructUnionBody(loc, tagType);
    record->recordName = name;
    record->isDef = tagUseKind == DEFINITION;
    ds.setTypeSpecType(tagType, loc, record);
}

void Parser::parseDeclarator(Declarator &d) {
    // This implements the 'declarator' production in the C grammar, then checks
    // for well-formedness and issues diagnostics.
    parseDeclaratorInternal(d, &Parser::parseDirectDeclarator);
}

DeclGroup *Parser::parseDeclaration(Declarator::TheContext context) {
}

/**
 * parseDeclaratorInternal - Parse a C declarator. The direct-declarator
 * is parsed by the function passed to it.
 */
void Parser::parseDeclaratorInternal(Declarator &d, void (Parser::*directDeclParser)(Declarator &)) {
    TokenKind tokKind = (*cursor)->getKind();
    // Not a pointer.
    if (tokKind != TokenKind::TK_STAR) {
        if (directDeclParser)
            (this->*directDeclParser)(d);
        return;
    }

    // Otherwise, '*' -> pointer.
    SourceLocation loc = (*cursor)->getLoc();
    ++cursor; // Consume the '*'.

    DeclSpec ds;
    parseTypeQualifierListOpt(ds);
    // Recursively parse the declarator.
    parseDeclaratorInternal(d, directDeclParser);
    // Remember that we parsed a pointer type, and remember the type-quals.
    d.declTypeInfo.push_back(DeclaratorChunk::getPointer(ds.typeQualifiers, loc));
}

void Parser::parseDirectDeclarator(Declarator &d) {
    if (isKind(cursor, TK_IDENTIFIER) && d.mayHaveIdentifier()) {
        d.setIdentifier((*cursor)->getStr(), (*cursor)->getLoc());
        ++cursor;
    } else if (isKind(cursor, TK_LPAR)) {
        // e.g.: "char (*x)" or "int (*y)(float)"
        parseParenDeclarator(d);
    } else if (d.mayOmitIdentifier()) {
        d.setIdentifier("", (*cursor)->getLoc());
    } else {
        if (d.context == Declarator::MEMBER_CONTEXT)
            error((*cursor)->getLoc(), "expected member name or semicolon");
        else
            error((*cursor)->getLoc(), "expected identifier after '('");
        d.setIdentifier("", (*cursor)->getLoc());
        d.invalidType = true;
    }

    while (true) {
        if (isKind(cursor, TK_LPAR)) {
            // TODO: ref Clang
        } else if (isKind(cursor, TK_LSQB)) {
            parseBracketDeclarator(d);
        } else {
            break;
        }
    }
}

DeclGroup *Parser::parseInitDeclaratorListAfterFirstDeclarator(Declarator &d) {}

void Parser::parseTypeQualifierListOpt(DeclSpec &ds) {
    while (true) {
        bool isInvalid = false;
        SourceLocation loc = (*cursor)->getLoc();

        switch ((*cursor)->getKind()) {
        case TK_CONST:
            isInvalid = ds.setTypeQual(DeclSpec::TQ_CONST, loc);
            break;
        case TK_VOLATILE:
            isInvalid = ds.setTypeQual(DeclSpec::TQ_VOLATILE, loc);
            break;
        case TK_RESTRICT:
            isInvalid = ds.setTypeQual(DeclSpec::TQ_RESTRICT, loc);
            break;
        default:
            ds.finish();
            return;
        }

        // If the specifier combination wasn't legal, issue a diagnostic.
        if (isInvalid) {
            error(loc, "illegal type qualifier combination");
        }
        ++cursor; // Remember to consume token.
    }
}

void Parser::parseParenDeclarator(Declarator &d) {
}

void Parser::parseBracketDeclarator(Declarator &d) {
}

RecordDecl *Parser::parseStructUnionBody(SourceLocation loc, DeclSpec::TST specType) {
    RecordDecl *rd = new RecordDecl(loc, specType == DeclSpec::TST_STRUCT);
    SourceLocation lBraceLoc = (*cursor)->getLoc();
    ++cursor;

    std::vector<FieldDeclarator> fieldDeclarators;

    while (!isKind(cursor, TK_RBRACE)) {
        // Each iteration of this loop reads one struct-declaration.
        DeclSpec ds;
        fieldDeclarators.clear();
        parseStructDeclaration(ds, fieldDeclarators);

        if (ds.typeSpecType == DeclSpec::TST_STRUCT || ds.typeSpecType == DeclSpec::TST_UNION) {
            RecordDecl *r = static_cast<RecordDecl *>(ds.typeRep);
            rd->fields.push_back(r);
        }

        for (auto &fd : fieldDeclarators) {
            QualType qt = getTypeForDeclarator(fd.d);
            FieldDecl *field = new FieldDecl(fd.d.identifierLoc, qt, fd.d.identifier, fd.bitFieldWidth);
            rd->fields.push_back(field);
        }

        if (isKind(cursor, TK_SEMI))
            ++cursor;
    }

    ++cursor; // Consume the '}'.
    return rd;
}

void Parser::parseStructDeclaration(DeclSpec &ds, std::vector<FieldDeclarator> &fields) {
    // Parse the common specifier-qualifiers-list piece.
    SourceLocation dsLoc = (*cursor)->getLoc();
    parseSpecifierQualifierList(ds);

    // A free-standing declaration specifier.
    if (isKind(cursor, TK_SEMI)) {
        warning(dsLoc, "declaration does not declare a member");
        return;
    }

    // Read struct-declarators until we find the semicolon.
    fields.emplace_back(ds);
    while (true) {
        FieldDeclarator &declaratorInfo = fields.back();

        // struct-declarator:
        //     declarator
        //     declarator{opt} ':' constant-expression
        if (!isKind(cursor, TK_COLON))
            parseDeclarator(declaratorInfo.d);
        if (isKind(cursor, TK_COLON)) {
            ++cursor;
            Expr *res = parseConditionalExpression();
            declaratorInfo.bitFieldWidth = res;
        }

        // Meet the end of struct-declarator-list.
        if (!isKind(cursor, TK_COMMA))
            return;

        ++cursor; // Consume the comma.

        fields.emplace_back(ds);
    }
}