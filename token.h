#ifndef _CRYOLITE_TOKEN_H_
#define _CRYOLITE_TOKEN_H_

#include <list>
#include <memory>
#include <set>
#include <string>

using HideSet = std::set<std::string>;

struct SourceLocation {
    const std::string *filename;
    unsigned int line;
    unsigned int column;
};

std::string srcLocToString(const SourceLocation &loc);

enum TokenKind {
#define TOK(X) TK_##X,
#include "tokenkind.def"
    NUM_TOKENS
};

class Token {
public:
    Token(const TokenKind &kind, SourceLocation &loc)
        : kind(kind), loc(loc) {}
    Token(const TokenKind &kind, SourceLocation &loc, const std::string &str)
        : kind(kind), loc(loc), str(str) {}

    ~Token() = default;

    TokenKind getKind();
    SourceLocation getLoc();
    std::string getStr();
    std::string toString();
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
    NumericLiteralParser(const std::string &str, SourceLocation Loc);

private:
};

#endif