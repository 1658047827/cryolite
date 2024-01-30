#include "parser.h"

TranslationUnit *Parser::ParseTranslationUnit() {
    TranslationUnit *transUnit = new TranslationUnit;
    auto begin = tksq.CBegin();
    return transUnit;
}

ExternalDeclaration *Parser::ParseExternalDeclaration() {
}