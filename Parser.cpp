#include "Parser.h"
#include "Diagnostic.h"
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
    typeRep = nullptr;
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

void Parser::initialize() {
    // Create the translation unit scope.  Install it as the current scope.
    assert(curScope == nullptr);
    enterScope(Scope::DECL_SCOPE);
    
}

bool Parser::skipUntil(const TokenKind *toks, unsigned numToks, bool stopAtSemi, bool dontConsume) {
    while (true) {
        for (unsigned i = 0; i != numToks; ++i) {
            if (tok.is(toks[i])) {
                if (!dontConsume)
                    consumeToken();
                return true;
            }
        }

        switch (tok.getKind()) {
        case TK_EOF:
            return false;
        case TK_SEMI:
            if (stopAtSemi)
                return false;
            // fall through
        default:
            consumeToken();
            break;
        }
    }
}

void Parser::enterScope(unsigned char scopeFlags) {
    if (numCachedScopes) {
        Scope *n = scopeCache[--numCachedScopes];
        n->init(curScope, scopeFlags);
        curScope = n;
    } else {
        curScope = new Scope(curScope, scopeFlags);
    }
}

void Parser::exitScope() {
    if (!curScope->declEmpty())
        sema.actOnPopScope(tok.getLoc(), curScope);

    Scope *old = curScope;
    curScope = old->getParent();

    if (numCachedScopes == scopeCacheSize)
        delete old;
    else
        scopeCache[numCachedScopes++] = old;
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
    switch (tok.getKind()) {
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
    SourceLocation loc = consumeToken();
    Expr *rhs = parseAssignmentExpression();
    // Break compound assignment into two BinaryExprs.
    // e.g. a += b => a = a + b
    if (op != BinaryOpKind::ASSIGN) {
        rhs = new BinaryExpr(loc, op, lhs, rhs);
    }
    return new BinaryExpr(loc, BinaryOpKind::ASSIGN, lhs, rhs);
}

Expr *Parser::parseConditionalExpression() {
    Expr *condExpr = parseLogicalOrExpression();
    if (tok.is(TK_QUESTION)) {
        SourceLocation loc = consumeToken();
        Expr *trueExpr = parseExpression();
        expect(cursor, TK_COLON);
        consumeToken();
        Expr *falseExpr = parseConditionalExpression();
        return new TernaryExpr(loc, condExpr, trueExpr, falseExpr);
    }
    return condExpr;
}

Expr *Parser::parseSimpleBinaryExpression(Expr *(Parser::*parseTerm)(), TokenKind kind, BinaryOpKind op) {
    Expr *lhs = (this->*parseTerm)();
    while (tok.is(kind)) {
        SourceLocation loc = consumeToken();
        Expr *rhs = (this->*parseTerm)();
        lhs = sema.actOnBinaryOp(curScope, loc, op, lhs, rhs);
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
        switch (tok.getKind()) {
        case TK_EQUALEQUAL:
            op = BinaryOpKind::EQUAL;
            break;
        case TK_EXCLAIMEQUAL:
            op = BinaryOpKind::NEQ;
            break;
        default:
            return lhs;
        }
        SourceLocation loc = consumeToken();
        Expr *rhs = parseRelationalExpression();
        lhs = new BinaryExpr(loc, op, lhs, rhs);
    }
}

Expr *Parser::parseRelationalExpression() {
    Expr *lhs = parseShiftExpression();
    while (true) {
        BinaryOpKind op;
        switch (tok.getKind()) {
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
        SourceLocation loc = consumeToken();
        Expr *rhs = parseShiftExpression();
        lhs = new BinaryExpr(loc, op, lhs, rhs);
    }
}

Expr *Parser::parseShiftExpression() {
    Expr *lhs = parseAdditiveExpression();
    while (true) {
        BinaryOpKind op;
        switch (tok.getKind()) {
        case TK_LESSLESS:
            op = BinaryOpKind::SHL;
            break;
        case TK_GREATERGREATER:
            op = BinaryOpKind::SHR;
            break;
        default:
            return lhs;
        }
        SourceLocation loc = consumeToken();
        Expr *rhs = parseAdditiveExpression();
        lhs = sema.actOnBinaryOp(curScope, loc, op, lhs, rhs);
    }
}

Expr *Parser::parseAdditiveExpression() {
    Expr *lhs = parseMultiplicativeExpression();
    while (true) {
        BinaryOpKind op;
        switch (tok.getKind()) {
        case TK_PLUS:
            op = BinaryOpKind::ADD;
            break;
        case TK_MINUS:
            op = BinaryOpKind::SUB;
            break;
        default:
            return lhs;
        }
        SourceLocation loc = consumeToken();
        Expr *rhs = parseMultiplicativeExpression();
        lhs = sema.actOnBinaryOp(curScope, loc, op, lhs, rhs);
    }
}

Expr *Parser::parseMultiplicativeExpression() {
    Expr *lhs = parseCastExpression();
    while (true) {
        BinaryOpKind op;
        switch (tok.getKind()) {
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
        SourceLocation loc = consumeToken();
        Expr *rhs = parseCastExpression();
        lhs = new BinaryExpr(loc, op, lhs, rhs);
    }
}

Expr *Parser::parseCastExpression() {
    TokenSeqConstIter nxt = std::next(cursor, 1);
    if (tok.is(TK_LPAR) && isFirstOfTypeName(nxt)) {
        // TODO
        return nullptr;
    } else {
        return parseUnaryExpression();
    }
}

Expr *Parser::parseUnaryExpression() {
    switch (tok.getKind()) {
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
    SourceLocation loc = consumeToken();
    Expr *operand = parseUnaryExpression();
    return new UnaryExpr(loc, op, operand);
}

UnaryExpr *Parser::parseUnaryOperatorExpression(UnaryOpKind op) {
    SourceLocation loc = consumeToken(); // Consume the unary operator.
    Expr *expr = parseCastExpression();
    return new UnaryExpr(loc, op, expr);
}

SizeofExpr *Parser::parseSizeof() {
    // SourceLocation loc = consumeToken(); // Consume the SIZEOF.
    // TokenSeqConstIter nxt = std::next(cursor, 1);
    // if (tok.is(TK_LPAR) && isFirstOfTypeName(nxt)) {
    //     consumeToken(); // Consume the '('.
    //     QualType qt = parseTypeName();
    //     expect(cursor, TK_RPAR);
    //     consumeToken(); // Consume the ')'.
    //     return new SizeofExpr(loc, qt);
    // } else {
    //     Expr *expr = parseUnaryExpression();
    //     return new SizeofExpr(loc, expr);
    // }
    return nullptr;
}

Expr *Parser::parsePostfixExpression() {
    // Postfix expression, which always starts with a compound literal or primary expression,
    // is followed by a series of suffixes like '++', '--', '->', '.' etc.
    Expr *expr;
    if (tok.is(TK_LPAR) && isFirstOfTypeName(cursor)) {
        expr = parseCompoundLiteral();
    } else {
        expr = parsePrimaryExpression();
    }
    return parsePostfixExpressionSuffix(expr);
}

Expr *Parser::parsePostfixExpressionSuffix(Expr *expr) {
    while (true) {
        switch (tok.getKind()) {
        case TK_LSQB:
        case TK_LPAR:
        case TK_DOT:
        case TK_ARROW:
        case TK_PLUSPLUS:
            expr = new UnaryExpr(tok.getLoc(), POSINC, expr);
            break;
        case TK_MINUSMINUS:
            expr = new UnaryExpr(tok.getLoc(), POSDEC, expr);
            break;
        default:
            return expr;
        }
        consumeToken();
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
    SourceLocation loc = tok.getLoc();
    switch (tok.getKind()) {
    case TK_IDENTIFIER:
        return parseIdentifier();
    case TK_NUMERIC_CONSTANT:
        ret = sema.actOnNumericConstant(tok);
        consumeToken();
        return ret;
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
    return nullptr;
}

Expr *Parser::parseParenthesesExpression() {
    expect(cursor, TK_LPAR);
    consumeToken();
    Expr *expr = parseExpression();
    expect(cursor, TK_RPAR);
    // Maintain consistency: after parseXXX, cursor always points to next token.
    consumeToken();
    return expr;
}

CharacterConstant *Parser::parseCharacterConstant() {
    SourceLocation loc = tok.getLoc();
    std::string s = tok.getStr(); // "'c'"
    unsigned int val;
    // TODO: Support more kinds of character constants.
    val = s[1];
    consumeToken(); // Maintain consistency.
    ArithType *arithType = ArithType::getArithType(ArithType::INT);
    return new CharacterConstant(loc, QualType(arithType), val);
}

StringLiteral *Parser::parseStringLiterals() {
    // TODO: Support more kinds of string literals.
    SourceLocation loc = tok.getLoc();
    std::string val;
    // Continuous string literals like "hello" " world" produces
    // a single string literal "hello world".
    while (tok.is(TK_STRING_LITERAL)) {
        std::string tmp = tok.getStr();
        // Remove "".
        tmp.erase(0, 1);
        tmp.pop_back();
        val.append(tmp);
        consumeToken();
    }
    std::size_t sz = val.size() + 1; // '\0'
    ArithType *charT = ArithType::getArithType(ArithType::CHAR_S);
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
        SourceLocation loc = tok.getLoc();

        switch (tok.getKind()) {
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

            consumeToken(); // Consume the identifier.

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
            consumeToken();
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
        consumeToken();
    }
}

void Parser::parseSpecifierQualifierList(DeclSpec &ds) {
    // specifier-qualifier-list is a subset of declaration-specifiers.
    // Just parse declaration-specifiers and complain about extra stuff.
    parseDeclarationSpecifiers(ds);

    unsigned specs = ds.getParsedSpecifiers();
    if (specs == DeclSpec::PQ_NONE) {
        error(tok.getLoc(), "require type specifier or type qualifier");
        ds.setTypeSpecError();
    }
    if (specs & DeclSpec::PQ_STORAGE_CLASS_SPECIFIER) {
        if (ds.storageClassSpecLoc.isValid())
            error(ds.storageClassSpecLoc, "storage class specifier is not allowed");
        ds.clearStorageClassSpecs();
    }
    if (specs & DeclSpec::PQ_FUNCTION_SPECIFIER) {
        if (ds.fsInlineSpecified)
            error(ds.fsInlineLoc, "function specifier is not allowed");
        ds.clearFunctionSpecs();
    }
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

    return QualType();
}

void Parser::parseEnumSpecifier(SourceLocation loc, DeclSpec &ds) {
}

void Parser::parseRecordSpecifier(SourceLocation loc, DeclSpec &ds) {
    DeclSpec::TST tagType;
    TokenKind k = tok.getKind();
    consumeToken(); // Consume struct-or-union.
    if (k == TK_STRUCT)
        tagType = DeclSpec::TST_STRUCT;
    else if (k == TK_UNION)
        tagType = DeclSpec::TST_UNION;
    else
        assert(0 && "Not a record specifier");

    // Parse the (optional) record name.
    std::string name;
    SourceLocation nameLoc;
    if (tok.is(TK_IDENTIFIER)) {
        name = tok.getStr();
        nameLoc = consumeToken();
    }

    // There are three options here.
    // If we have 'struct foo;', then this is a forward declaration.
    // If we have 'struct foo {...' then this is a definition.
    // Otherwise we have something like 'struct foo x', a reference.
    TagUseKind tagUseKind;
    if (tok.is(TK_LBRACE))
        tagUseKind = DEFINITION;
    else if (tok.is(TK_SEMI))
        tagUseKind = DECLARATION;
    else
        tagUseKind = REFERENCE;

    if (name.empty() && tagUseKind != TagUseKind::DEFINITION) {
        error(loc, "declaration of anonymous record must be a definition");
        // Skip the rest of this declarator, up until the comma or semicolon.
        skipUntil(TK_COMMA, true);
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
    return nullptr;
}

/**
 * parseDeclaratorInternal - Parse a C declarator. The direct-declarator
 * is parsed by the function passed to it.
 */
void Parser::parseDeclaratorInternal(Declarator &d, void (Parser::*directDeclParser)(Declarator &)) {
    TokenKind tokKind = tok.getKind();
    // Not a pointer.
    if (tokKind != TokenKind::TK_STAR) {
        if (directDeclParser)
            (this->*directDeclParser)(d);
        return;
    }

    // Otherwise, '*' -> pointer.
    SourceLocation loc = consumeToken(); // Consume the '*'.

    DeclSpec ds;
    parseTypeQualifierListOpt(ds);
    // Recursively parse the declarator.
    parseDeclaratorInternal(d, directDeclParser);
    // Remember that we parsed a pointer type, and remember the type-quals.
    d.declTypeInfo.push_back(DeclaratorChunk::getPointer(ds.typeQualifiers, loc));
}

void Parser::parseDirectDeclarator(Declarator &d) {
    if (tok.is(TK_IDENTIFIER) && d.mayHaveIdentifier()) {
        d.setIdentifier(tok.getStr(), tok.getLoc());
        consumeToken();
    } else if (tok.is(TK_LPAR)) {
        // e.g. "char (*x)" or "int (*y)(float)"
        parseParenDeclarator(d);
    } else if (d.mayOmitIdentifier()) {
        d.setIdentifier("", tok.getLoc());
    } else {
        if (d.context == Declarator::MEMBER_CONTEXT)
            error(tok.getLoc(), "expected member name or semicolon");
        else
            error(tok.getLoc(), "expected identifier after '('");
        d.setIdentifier("", tok.getLoc());
        d.invalidType = true;
    }

    while (true) {
        if (tok.is(TK_LPAR)) {
            // TODO: ref Clang
        } else if (tok.is(TK_LSQB)) {
            parseBracketDeclarator(d);
        } else {
            break;
        }
    }
}

DeclGroup *Parser::parseInitDeclaratorListAfterFirstDeclarator(Declarator &d) {
    return nullptr;
}

void Parser::parseTypeQualifierListOpt(DeclSpec &ds) {
    while (true) {
        bool isInvalid = false;
        SourceLocation loc = tok.getLoc();

        switch (tok.getKind()) {
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
        consumeToken(); // Remember to consume token.
    }
}

void Parser::parseParenDeclarator(Declarator &d) {
}

void Parser::parseBracketDeclarator(Declarator &d) {
}

RecordDecl *Parser::parseStructUnionBody(SourceLocation loc, DeclSpec::TST specType) {
    RecordDecl *rd = new RecordDecl(loc, specType == DeclSpec::TST_STRUCT);
    SourceLocation lBraceLoc = consumeToken();
    std::vector<FieldDeclarator> fieldDeclarators;

    while (tok.isNot(TK_RBRACE)) {
        // Each iteration of this loop reads one struct-declaration.
        DeclSpec ds;
        fieldDeclarators.clear();
        parseStructDeclaration(ds, fieldDeclarators);

        for (auto &fd : fieldDeclarators) {
            QualType qt = getTypeForDeclarator(fd.d);
            FieldDecl *field = new FieldDecl(fd.d.identifierLoc, qt, fd.d.identifier, fd.bitFieldWidth);
            rd->fields.push_back(field);
        }

        if (tok.is(TK_SEMI)) {
            consumeToken();
        } else if (tok.is(TK_RBRACE)) {
            error(tok.getLoc(), "expected ';' at end of declaration list");
            break;
        } else {
            error(tok.getLoc(), "expected ';' at end of declaration list");
            skipUntil(TK_RBRACE, true, true);
        }
    }

    consumeToken(); // Consume the '}'.
    return rd;
}

void Parser::parseStructDeclaration(DeclSpec &ds, std::vector<FieldDeclarator> &fields) {
    // Parse the common specifier-qualifiers-list piece.
    SourceLocation dsLoc = tok.getLoc();
    parseSpecifierQualifierList(ds);

    // A free-standing declaration specifier.
    if (tok.is(TK_SEMI)) {
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
        if (tok.isNot(TK_COLON))
            parseDeclarator(declaratorInfo.d);
        if (tok.is(TK_COLON)) {
            consumeToken();
            Expr *res = parseConditionalExpression();
            // TODO: Check if this expr is constant.
            declaratorInfo.bitFieldWidth = res;
        }

        // Meet the end of struct-declarator-list.
        if (tok.isNot(TK_COMMA))
            return;

        consumeToken(); // Consume the comma.

        fields.emplace_back(ds);
    }
}