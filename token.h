#ifndef _CRYOLITE_TOKEN_H_
#define _CRYOLITE_TOKEN_H_

#include <list>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>

using HideSet = std::set<std::string>;

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
#include "tokenkind.def"
    NUM_TOKENS
};

static const std::unordered_map<TokenKind, std::string> tokenKind2Str{
#define PUNCTUATOR(X, Y) {TK_##X, Y},
#define KEYWORD(X, Y) {TK_##X, Y},
#include "tokenkind.def"
};

class Token {
public:
    Token() : kind(TokenKind::TK_UNKNOWN) {}
    Token(const TokenKind &kind, SourceLocation &loc) : kind(kind), loc(loc) {}
    Token(const TokenKind &kind, SourceLocation &loc, const std::string &str)
        : kind(kind), loc(loc), str(str) {}

    ~Token() = default;

    TokenKind getKind();
    SourceLocation getLoc();
    std::string getStr();
    std::string toString();

    bool is(TokenKind k) const { return kind == k; }
    bool isNot(TokenKind k) const { return kind != k; }

    void hideSetAdd(std::string &tokenStr);
    bool hideSetHas(std::string &tokenStr);

private:
    TokenKind kind;
    SourceLocation loc;
    std::string str;
    HideSet hideSet;
};

HideSet hideSetIntersection(HideSet &lhs, HideSet &rhs);
HideSet hideSetUnion(HideSet &lhs, HideSet &rhs);

using TokenSeqIter = std::list<std::unique_ptr<Token>>::iterator;
using TokenSeqConstIter = std::list<std::unique_ptr<Token>>::const_iterator;

class TokenSequence {
public:
    TokenSequence() = default;

    ~TokenSequence() = default;

    template <typename... Args>
    void emplaceBack(Args &&...args) {
        tokenList.emplace_back(std::make_unique<Token>(std::forward<Args>(args)...));
    }
    void pushBack(Token &token);
    void print(std::ostream &out);
    TokenSeqIter begin();
    TokenSeqIter end();
    TokenSeqConstIter cBegin();
    TokenSeqConstIter cEnd();

private:
    std::list<std::unique_ptr<Token>> tokenList;
};

// NumericLiteralParser - Analysis and classify a string as either integer, floating, or erroneous.
class NumericLiteralParser {
public:
    NumericLiteralParser(const char *begin, const char *end, const SourceLocation &loc);

    bool hadError;
    bool isUnsigned;
    bool isLong; // This is *not* set for long long.
    bool isLongLong;
    bool isFloat;

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