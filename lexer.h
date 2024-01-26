#ifndef _CRYOLITH_LEXER_H_
#define _CRYOLITH_LEXER_H_

#include "token.h"

class Lexer {
public:
    Lexer(std::string *filename, std::vector<char> &buffer)
        : curLoc{filename, 1, 1}, buffer(buffer), p(0) {}

    ~Lexer() = default;

    void Lex(TokenSequence &ts);

private:
    char CurChar();
    char Next();
    char LookAhead(size_t n = 1);
    bool NextIs(char c);
    bool Try(char c);

    void SkipComment();
    Token LexNumericConstant();
    Token LexCharConstant();
    Token LexStringLiteral();
    // LexIdentifier - Lex identifier and C keyword.
    Token LexIdentifier();

    SourceLocation curLoc;
    std::vector<char> &buffer;
    size_t p;
};

int ReadFile(std::string &filename, std::vector<char> &buffer);

#endif