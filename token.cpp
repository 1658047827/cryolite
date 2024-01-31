#include "token.h"
#include <algorithm>
#include <iostream>
#include <unordered_map>

static const std::unordered_map<TokenKind, std::string> tokenKind2Str{
#define PUNCTUATOR(X, Y) {TK_ ## X, Y},
#define KEYWORD(X, Y) {TK_ ## X, Y},
#include "tokenkind.def"
};

std::string SrcLocToString(SourceLocation &loc) {
    std::string s(*loc.filename);
    s += ":";
    s += std::to_string(loc.line);
    s += ":";
    s += std::to_string(loc.column);
    s += ": ";
    return s;
}

TokenKind Token::GetKind() {
    return kind;
}

SourceLocation Token::GetLoc() {
    return loc;
}

std::string Token::GetStr() {
    return str;
}

std::string Token::ToString() {
    if (str.empty()) {
        auto iter = tokenKind2Str.find(kind);
        if (iter != tokenKind2Str.cend())
            return iter->second;
        return "";
    } else {
        return str;
    }
}

void Token::HideSetAdd(std::string &tokenStr) {
    hideSet.insert(tokenStr);
}

bool Token::HideSetHas(std::string &tokenStr) {
    return hideSet.count(tokenStr) == 1;
}

HideSet HideSetIntersection(HideSet &lhs, HideSet &rhs) {
    HideSet hs;
    std::set_intersection(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend(), std::inserter(hs, hs.begin()));
    return hs;
}

HideSet HideSetUnion(HideSet &lhs, HideSet &rhs) {
    HideSet hs;
    std::set_union(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend(), std::inserter(hs, hs.begin()));
    return hs;
}

void TokenSequence::PushBack(Token &token) {
    tokenList.push_back(std::make_unique<Token>(token));
}

void TokenSequence::Print(std::ostream &out) {
    for (auto &tkp : tokenList) {
        out << tkp->ToString() << ' ';
    }
    out << std::endl;
}

std::list<std::unique_ptr<Token>>::iterator TokenSequence::Begin() {
    return tokenList.begin();
}

std::list<std::unique_ptr<Token>>::iterator TokenSequence::End() {
    return tokenList.end();
}

std::list<std::unique_ptr<Token>>::const_iterator TokenSequence::CBegin() {
    return tokenList.cbegin();
}

std::list<std::unique_ptr<Token>>::const_iterator TokenSequence::CEnd() {
    return tokenList.cend();
}

NumericLiteralParser::NumericLiteralParser(const std::string &str, SourceLocation Loc) {
}