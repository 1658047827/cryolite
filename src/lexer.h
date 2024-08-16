#ifndef _CRYOLITE_LEXER_H_
#define _CRYOLITE_LEXER_H_

#include "token.h"

typedef struct Lexer {
    const char *bufferStart;
    const char *bufferEnd;
    const char *bufferPtr;
} Lexer;

void initLexer(Lexer *lexer);
void lex(Lexer *lexer, Token *result);
void lexTokenInternal(Lexer *lexer, Token *result);
char getCharAndSize(const char *ptr, unsigned *size);
char getCharAndSizeSlow(const char* ptr, unsigned *size, Token *tok);
const char *consumeChar(const char *ptr, unsigned int size, Token *tok);
char getAndAdvanceChar(const char **ptr, Token *tok);
void formTokenWithChars(Lexer *lexer, Token *result, const char *tokEnd, TokenKind kind);

void lexIdentifier(Token *result, const char *curPtr);
void lexNumericConstant(Lexer *lexer, Token *result, const char *curPtr);

void skipWhitespace(Lexer *lexer, const char *curPtr);
void skipBCPLComment();
void skipBlockComment();

#endif