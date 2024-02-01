#ifndef _CRYOLITE_PARSER_H_
#define _CRYOLITE_PARSER_H_

#include "syntax.h"
#include "token.h"
#include <bitset>

using TokenBitSet = std::bitset<TokenKind::NUM_TOKENS>;

class Parser {
public:
    Parser(TokenSequence &ts)
        : tksq(ts), cursor(ts.CBegin()) { InitBitSet(); }

    TranslationUnit *ParseTranslationUnit();
    ExternalDeclaration *ParseExternalDeclaration();
    DeclSpecifiers *ParseDeclarationSpecifiers();
    Declarator *ParseDeclarator();
    StructOrUnionSpecifier *ParseStructOrUnionSpecifier();
    EnumSpecifier *ParseEnumSpecifier();
    StructDeclaration *ParseStructDeclaration();
    SpecifierQualifier *ParseSpecifierQualifier();
    StructDeclarator *ParseStructDeclarator();
    ConditionalExpr* ParseConditionalExpr();

private:
    void InitBitSet();
    template <typename... Args>
    void SetBitSet(TokenBitSet &bitset, Args... values) {
        (bitset.set(values), ...);
    }
    bool IsInBitSet(TokenBitSet &bitset, TokenSeqConstIter &iter);

    TokenSequence &tksq;
    TokenSeqConstIter cursor;

    TokenBitSet firstOfSpecifierQualifier;
    TokenBitSet firstOfDeclarator;
};

#endif