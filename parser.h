#ifndef _CRYOLITE_PARSER_H_
#define _CRYOLITE_PARSER_H_

#include "ast.h"
#include "token.h"
#include <bitset>

using TokenBitSet = std::bitset<NUM_TOKENS>;

class Scope {
};

class Parser {
public:
    Parser(TokenSequence &ts)
        : tksq(ts), cursor(ts.cBegin()) { initBitSet(); }

    /**
     * Expressions
     */
    Expr *parseExpression();
    Expr *parseAssignmentExpression();
    Expr *parseConditionalExpression();
    // parseSimpleBinaryExpression - Parse very simple binary expressions like
    // LogicalAndExpression, LogicalOrExpression, BitOrExpression etc.
    // They only involve one type of binary operator.
    Expr *parseSimpleBinaryExpression(Expr *(Parser::*parseTerm)(), TokenKind kind, BinaryOpKind op);
    Expr *parseLogicalOrExpression();
    Expr *parseLogicalAndExpression();
    Expr *parseBitOrExpression();
    Expr *parseBitXorExpression();
    Expr *parseBitAndExpression();
    Expr *parseEqualityExpression();
    Expr *parseRelationalExpression();
    Expr *parseShiftExpression();
    Expr *parseAdditiveExpression();
    Expr *parseMultiplicativeExpression();
    Expr *parseCastExpression();
    Expr *parseUnaryExpression();
    UnaryExpr *parsePrefixIncrementOrDecrement(UnaryOpKind op);
    UnaryExpr *parseUnaryOperatorExpression(UnaryOpKind op);
    Expr *parsePostfixExpression();

    Expr *parsePrimaryExpr();
    // Expr *parseParenthesesExpression();
    // IntegerConstant *parseIntegerConstant();
    // FloatingConstant *parseFloatingConstant();
    // StringLiteral *parseStringLiteral();

    /**
     * Declarations
     */
    // void *parseSpecifierQualifierList();
    // void *parseTypeName();

    /**
     * Statements and blocks
     */

    /**
     * External definitions
     */
    TransUnit *parseTranslationUnit();

private:
    inline bool isKind(TokenSeqConstIter &iter, TokenKind kind);
    void expect(TokenSeqConstIter &iter, TokenKind kind);
    void initBitSet();
    template <typename... Args>
    void setBitSet(TokenBitSet &bitset, Args... values) {
        (bitset.set(values), ...);
    }
    bool isInBitSet(TokenBitSet &bitset, TokenSeqConstIter &iter);
    bool isFirstOfTypeName(TokenSeqConstIter &iter);

    TokenSequence &tksq;
    TokenSeqConstIter cursor;

    TokenBitSet firstOfSpecifierQualifier;
    TokenBitSet firstOfDeclarator;
};

#endif