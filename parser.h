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

    TransUnit *parseTranslationUnit();
    ExternalDeclaration *parseExternalDeclaration();
    DeclarationSpecifiers *parseDeclarationSpecifiers();
    Declarator *parseDeclarator();
    StructOrUnionSpecifier *parseStructOrUnionSpecifier();
    EnumSpecifier *parseEnumSpecifier();
    StructDeclaration *parseStructDeclaration();
    SpecifierQualifier *parseSpecifierQualifier();
    StructDeclarator *parseStructDeclarator();
    ConditionalExpression *parseConditionalExpr();
    LogicalOrExpression *parseLogicalOrExpr();
    LogicalAndExpression *parseLogicalAndExpr();
    BitOrExpression *parseBitOrExpr();
    BitXorExpression *parseBitXorExpr();
    BitAndExpression *parseBitAndExpr();
    EqualityExpression *parseEqualityExpr();
    RelationalExpression *parseRelationalExpr();
    Expr *parseShiftExpression();
    Expr *parseAdditiveExpression();
    Expr *parseMultiplicativeExpression();
    Expr *parseCastExpression();
    UnaryExpression *parseUnaryExpr();
    Expression *parseExpression();
    TypeName *parseTypeName();
    InitializerList *parseInitializerList();

private:
    inline bool isKind(TokenSeqConstIter &iter, TokenKind kind);
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