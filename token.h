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

std::string SrcLocToString(SourceLocation &loc);

enum TokenKind {
#define TOK(X) TK_ ## X,
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

    TokenKind GetKind();
    SourceLocation GetLoc();
    std::string GetStr();
    std::string ToString();
    void HideSetAdd(std::string &tokenStr);
    bool HideSetHas(std::string &tokenStr);

private:
    TokenKind kind;
    SourceLocation loc;
    std::string str;
    HideSet hideSet;
};

HideSet HideSetIntersection(HideSet &lhs, HideSet &rhs);
HideSet HideSetUnion(HideSet &lhs, HideSet &rhs);

class TokenSequence {
public:
    TokenSequence() = default;

    ~TokenSequence() = default;

    template <typename... Args>
    void EmplaceBack(Args &&...args) {
        tokenList.emplace_back(std::make_unique<Token>(std::forward<Args>(args)...));
    }
    void PushBack(Token &token);
    void Print(std::ostream &out);
    std::list<std::unique_ptr<Token>>::iterator Begin();
    std::list<std::unique_ptr<Token>>::iterator End();
    std::list<std::unique_ptr<Token>>::const_iterator CBegin();
    std::list<std::unique_ptr<Token>>::const_iterator CEnd();

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