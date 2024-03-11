#ifndef _CRYOLITE_LEXER_H_
#define _CRYOLITE_LEXER_H_

#include "Token.h"
#include <vector>

class Cursor {
public:
    Cursor(std::string *filename, std::vector<char> &buf)
        : loc{filename, 1, 1}, iter(buf.cbegin()) {}

    SourceLocation getLoc() const { return loc; }

    Cursor &operator++() {
        if (*iter == '\n') {
            loc.line += 1;
            loc.column = 1;
        } else {
            loc.column += 1;
        }
        ++iter;
        return *this;
    }

    Cursor operator++(int) {
        Cursor tmp = *this;
        ++(*this);
        return tmp;
    }

    const char &operator*() {
        return *iter;
    }

    unsigned operator-(const Cursor &other) const {
        return iter - other.iter;
    }

    const char *base() const {
        return iter.base();
    }

private:
    SourceLocation loc;
    std::vector<char>::const_iterator iter;
};

class Lexer {
public:
    Lexer(std::string *filename, std::vector<char> &buf)
        : buffer(buf), bufferCursor(filename, buf) {}

    ~Lexer() = default;

    void lex(Token &tok);

private:
    inline char getAndAdvance(Cursor &cursor) { return *cursor++; }

    void formTokenWithChars(Token &tok, Cursor tokEnd, TokenKind kind) {
        unsigned tokLen = tokEnd - bufferCursor;
        tok.setLength(tokLen);
        tok.setLoc(bufferCursor.getLoc());
        tok.setKind(kind);
        bufferCursor = tokEnd;
    }

    char lookAhead(std::size_t n = 1);
    bool nextIs(char c);
    bool tryNext(char c);

    void skipComment();
    void lexNumericConstant(Token &tok, Cursor curCursor);
    void lexCharConstant(Token &tok);
    void lexStringLiteral(Token &tok);
    // lexIdentifier - Lex identifier and C keyword.
    void lexIdentifier(Token &tok);

    std::vector<char> &buffer;
    Cursor bufferCursor;
};

// readFile - Read file content into buffer.
std::vector<char> readFile(std::string &filename);

#endif