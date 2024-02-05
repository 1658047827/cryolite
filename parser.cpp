#include "parser.h"
#include "diagnostic.h"
#include <cassert>

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
 * assignment-operator:
 *     =
 *     *=
 *     /=
 *     %=
 *     +=
 *     -=
 *     <<=
 *     >>=
 *     &=
 *     ^=
 *     |=
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
    case TK_STAR:
    case TK_PLUS:
    case TK_MINUS:
    case TK_TILDE:
    case TK_EXCLAIM:
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
    return nullptr;
}

Expr *Parser::parsePostfixExpression() {
    return parsePrimaryExpr();
}

Expr *Parser::parsePrimaryExpr() {
    Expr* ret;
    switch ((*cursor)->getKind()) {
    case TK_IDENTIFIER:
    case TK_NUMERIC_CONSTANT:
        ret = new IntegerConstant((*cursor)->getLoc(), std::stoull((*cursor)->getStr()));
        ++cursor;
        return ret;
    case TK_CHAR_CONSTANT:
    case TK_STRING_LITERAL:
    case TK_LPAR:
    default:
        // error();
        return nullptr;
    }
}