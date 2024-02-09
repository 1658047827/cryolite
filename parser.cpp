#include "parser.h"
#include "diagnostic.h"
#include <cassert>

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
        QualType *qt = parseTypeName();
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

QualType *Parser::parseTypeName() {
    return nullptr;
}