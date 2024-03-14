#ifndef _CRYOLITE_LEXER_H_
#define _CRYOLITE_LEXER_H_

#include "Identifier.h"
#include "Token.h"
#include <vector>

class Cursor {
public:
    Cursor(std::string *filename, std::vector<char> &buf);

    SourceLocation getLoc() const;
    const char *getPtr() const;

    Cursor &operator++();
    Cursor operator++(int);

    const char &operator*();

    Cursor operator+(int distance) const;

    unsigned operator-(const Cursor &other) const;

private:
    SourceLocation loc;
    const char *ptr;
};

class Lexer {
public:
    Lexer(std::string *filename, std::vector<char> &buf, IdentifierTable &idTable);

    ~Lexer() = default;

    void lex(Token &tok);

private:
    void formTokenWithChars(Token &tok, Cursor tokEnd, TokenKind kind);

    void skipWhiteSpace(Cursor curCursor);
    void skipBCPLComment(Cursor curCursor);
    void skipBlockComment(Cursor curCursor);

    void lexNumericConstant(Token &tok, Cursor curCursor);
    void lexCharConstant(Token &tok, Cursor curCursor);
    void lexStringLiteral(Token &tok, Cursor curCursor);
    void lexIdentifier(Token &tok, Cursor curCursor);

    IdentifierInfo *lookUpIdentifierInfo(Token &tok, const char *bufStart);

    IdentifierTable &identifierTable;
    std::vector<char> &buffer;
    Cursor bufferCursor;
};

// readFile - Read file content into buffer.
std::vector<char> readFile(std::string &filename);

#endif