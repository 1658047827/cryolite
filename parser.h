#ifndef _CRYOLITH_PARSER_H_
#define _CRYOLITH_PARSER_H_

#include "syntax.h"
#include "token.h"

class Parser {
public:
    Parser(TokenSequence &ts)
        : tksq(ts) {}

    TranslationUnit *ParseTranslationUnit();
    ExternalDeclaration* ParseExternalDeclaration();

private:
    TokenSequence &tksq;
};

#endif