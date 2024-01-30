#ifndef _CRYOLITE_PARSER_H_
#define _CRYOLITE_PARSER_H_

#include "syntax.h"
#include "token.h"

class Parser {
public:
    Parser(TokenSequence &ts)
        : tksq(ts), cursor(ts.CBegin()) {}

    TranslationUnit *ParseTranslationUnit();
    ExternalDeclaration *ParseExternalDeclaration();

private:
    TokenSequence &tksq;
    std::list<std::unique_ptr<Token>>::const_iterator cursor;
};

#endif