#ifndef _CRYOLITE_LEXER_H_
#define _CRYOLITE_LEXER_H_

#include "token.h"
#include <vector>

class Lexer {
public:
    Lexer(std::string *filename, std::vector<char> &buffer)
        : curLoc{filename, 1, 1}, buffer(buffer), p(0) {}

    ~Lexer() = default;

    void lex(TokenSequence &ts);

private:
    char curChar();
    char next();
    char lookAhead(size_t n = 1);
    bool nextIs(char c);
    bool tryNext(char c);

    void skipComment();
    Token lexNumericConstant();
    Token lexCharConstant();
    Token lexStringLiteral();
    // lexIdentifier - Lex identifier and C keyword.
    Token lexIdentifier();

    SourceLocation curLoc;
    std::vector<char> &buffer;
    size_t p;
};

int readFile(std::string &filename, std::vector<char> &buffer);

#endif