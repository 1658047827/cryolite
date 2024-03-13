#ifndef _CRYOLITE_TOKEN_H_
#define _CRYOLITE_TOKEN_H_

#include <cassert>
#include <list>
#include <set>
#include <string>

// class IdentifierInfo;

struct SourceLocation {
    const std::string *filename;
    unsigned int line;
    unsigned int column;

    bool isValid() const { return filename != nullptr && line >= 0 && column >= 0; }
};

std::string srcLocToStr(const SourceLocation &loc);
std::string srcLocToPos(const SourceLocation &loc);

enum TokenKind {
#define TOK(X) TK_##X,
#include "TokenKind.def"
    NUM_TOKENS
};

std::string_view getTokenSpelling(TokenKind kind);

class Token {
public:
    Token() : kind(TokenKind::TK_UNKNOWN) {}
    Token(const TokenKind &kind, SourceLocation &loc) : kind(kind), loc(loc) {}

    ~Token() = default;

    TokenKind getKind() const { return kind; }
    SourceLocation getLoc() const { return loc; }
    unsigned getLength() const { return length; }

    void setKind(TokenKind k) { kind = k; }
    void setLoc(SourceLocation l) { loc = l; }
    void setLength(unsigned len) { length = len; }

    void setLiteralPtr(const char *p) {
        assert(isLiteral() && "cannot set literal data of non-literal");
        ptr = (void *)p;
    }

    // IdentifierInfo *getIdentifierInfo() const {
    //     if (isLiteral())
    //         return nullptr;
    //     return (IdentifierInfo *)ptr;
    // }

    void clear() {
        length = 0;
        kind = TokenKind::TK_UNKNOWN;
        ptr = nullptr;
        loc = SourceLocation();
    }

    bool is(TokenKind k) const { return kind == k; }
    bool isNot(TokenKind k) const { return kind != k; }

    bool isLiteral() const {
        return kind >= TokenKind::TK_NUMERIC_CONSTANT &&
               kind <= TokenKind::TK_STRING_LITERAL;
    }

    std::string_view repr() {
        if (kind == TokenKind::TK_IDENTIFIER) {

        } else if (isLiteral()) {
            return std::string_view((const char *)ptr, length);
        } else {
            return getTokenSpelling(kind);
        }
        return std::string_view();
    }

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

    bool isIntegerLiteral() const {
        return !sawPeriod && !sawExponent;
    }
    bool isFloatingLiteral() const {
        return sawPeriod || sawExponent;
    }
    bool hasSuffix() const {
        return suffixBegin != thisTokEnd;
    }

    unsigned getRadix() const { return radix; }

private:
    const char *const thisTokBegin;
    const char *const thisTokEnd;
    const char *digitsBegin, *suffixBegin; // markers
    const char *s;                         // cursor

    unsigned radix;

    bool sawExponent, sawPeriod;

    void parseNumberStartingWithZero(SourceLocation TokLoc);

    // skipHexDigits - Read and skip over any hex digits, up to End.
    // Return a pointer to the first non-hex digit or End.
    const char *skipHexDigits(const char *ptr) {
        while (ptr != thisTokEnd && std::isxdigit(*ptr))
            ++ptr;
        return ptr;
    }

    // skipOctalDigits - Read and skip over any octal digits, up to End.
    // Return a pointer to the first non-hex digit or End.
    const char *skipOctalDigits(const char *ptr) {
        while (ptr != thisTokEnd && ((*ptr >= '0') && (*ptr <= '7')))
            ++ptr;
        return ptr;
    }

    // skipDigits - Read and skip over any digits, up to End.
    // Return a pointer to the first non-hex digit or End.
    const char *skipDigits(const char *ptr) {
        while (ptr != thisTokEnd && std::isdigit(*ptr))
            ++ptr;
        return ptr;
    }

    // skipBinaryDigits - Read and skip over any binary digits, up to End.
    // Return a pointer to the first non-binary digit or End.
    const char *SkipBinaryDigits(const char *ptr) {
        while (ptr != thisTokEnd && (*ptr == '0' || *ptr == '1'))
            ++ptr;
        return ptr;
    }
};

#endif