#ifndef _CRYOLITE_TOKEN_H_
#define _CRYOLITE_TOKEN_H_

#include "TokenKind.h"
#include <cassert>
#include <list>
#include <set>
#include <string>

class IdentifierInfo;

struct SourceLocation {
    const std::string *filename;
    unsigned int line;
    unsigned int column;

    bool isValid() const { return filename != nullptr && line >= 0 && column >= 0; }
};

std::string srcLocToStr(const SourceLocation &loc);
std::string srcLocToPos(const SourceLocation &loc);

class Token {
public:
    Token();
    Token(const TokenKind &kind, SourceLocation &loc);

    ~Token() = default;

    TokenKind getKind() const;
    SourceLocation getLoc() const;
    unsigned getLength() const;
    IdentifierInfo *getIdentifierInfo() const;

    void setKind(TokenKind k);
    void setLoc(SourceLocation l);
    void setLength(unsigned len);
    void setLiteralPtr(const char *p);
    void setIdentifierInfo(IdentifierInfo *p);

    void clear();

    bool is(TokenKind k) const;
    bool isNot(TokenKind k) const;
    bool isLiteral() const;
    bool isKeyword() const;

    std::string_view repr() const;

private:
    TokenKind kind;
    SourceLocation loc;

    // ptr - An union of three different pointer types, which depends
    // on what type of token this is:
    // Identifier:
    //     This is an IdentifierInfo*, which contains the uniqued identifier spelling.
    // Literal:
    //     This is a pointer to the start of the token in a text buffer.
    // Other:
    //     This is null.
    void *ptr;

    // length - The number of bytes of the token.
    unsigned length;
};

// NumericLiteralParser - Analysis and classify a string as either integer, floating, or erroneous.
class NumericLiteralParser {
public:
    NumericLiteralParser(const char *begin, const char *end, const SourceLocation &loc);

    bool hadError;
    bool isUnsigned; // Set for u suffix.
    bool isLong;     // Set for l suffix. This is *not* set for long long.
    bool isLongLong; // Set for ll suffix.
    bool isFloat;    // Set for f suffix.

    bool isIntegerLiteral() const;
    bool isFloatingLiteral() const;
    bool hasSuffix() const;

    unsigned getRadix() const;

private:
    const char *const thisTokBegin;
    const char *const thisTokEnd;
    const char *digitsBegin, *suffixBegin; // markers
    const char *s;                         // cursor

    unsigned radix;

    bool sawExponent, sawPeriod;

    void parseNumberStartingWithZero(SourceLocation TokLoc);

    const char *skipHexDigits(const char *ptr);
    const char *skipOctalDigits(const char *ptr);
    const char *skipDigits(const char *ptr);
    const char *skipBinaryDigits(const char *ptr);
};

#endif