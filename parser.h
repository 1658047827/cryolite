#ifndef _CRYOLITE_PARSER_H_
#define _CRYOLITE_PARSER_H_

#include "syntax.h"
#include "token.h"
#include <bitset>

using TokenBitSet = std::bitset<TokenKind::NUM_TOKENS>;

class Parser {
public:
    Parser(TokenSequence &ts)
        : tksq(ts), cursor(ts.cBegin()) { initBitSet(); }

    TranslationUnit *parseTranslationUnit();
    ExternalDeclaration *parseExternalDeclaration();
    DeclSpecifiers *parseDeclarationSpecifiers();
    Declarator *parseDeclarator();
    StructOrUnionSpecifier *parseStructOrUnionSpecifier();
    EnumSpecifier *parseEnumSpecifier();
    StructDeclaration *parseStructDeclaration();
    SpecifierQualifier *parseSpecifierQualifier();
    StructDeclarator *parseStructDeclarator();
    ConditionalExpr* parseConditionalExpr();

private:
    void initBitSet();
    template <typename... Args>
    void setBitSet(TokenBitSet &bitset, Args... values) {
        (bitset.set(values), ...);
    }
    bool isInBitSet(TokenBitSet &bitset, TokenSeqConstIter &iter);

    TokenSequence &tksq;
    TokenSeqConstIter cursor;

    TokenBitSet firstOfSpecifierQualifier;
    TokenBitSet firstOfDeclarator;
};

#endif