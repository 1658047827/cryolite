#include "token.h"
#include <algorithm>
#include <iostream>

std::string srcLocToString(const SourceLocation &loc) {
    std::string s(*loc.filename);
    s += ":";
    s += std::to_string(loc.line);
    s += ":";
    s += std::to_string(loc.column);
    s += ": ";
    return s;
}

TokenKind Token::getKind() {
    return kind;
}

SourceLocation Token::getLoc() {
    return loc;
}

std::string Token::getStr() {
    return str;
}

std::string Token::toString() {
    if (str.empty()) {
        auto iter = tokenKind2Str.find(kind);
        if (iter != tokenKind2Str.cend())
            return iter->second;
        return "";
    } else {
        return str;
    }
}

void Token::hideSetAdd(std::string &tokenStr) {
    hideSet.insert(tokenStr);
}

bool Token::hideSetHas(std::string &tokenStr) {
    return hideSet.count(tokenStr) == 1;
}

HideSet hideSetIntersection(HideSet &lhs, HideSet &rhs) {
    HideSet hs;
    std::set_intersection(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend(), std::inserter(hs, hs.begin()));
    return hs;
}

HideSet hideSetUnion(HideSet &lhs, HideSet &rhs) {
    HideSet hs;
    std::set_union(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend(), std::inserter(hs, hs.begin()));
    return hs;
}

void TokenSequence::pushBack(Token &token) {
    tokenList.push_back(std::make_unique<Token>(token));
}

void TokenSequence::print(std::ostream &out) {
    for (auto &tkp : tokenList) {
        out << tkp->toString() << ' ';
    }
    out << std::endl;
}

TokenSeqIter TokenSequence::begin() {
    return tokenList.begin();
}

TokenSeqIter TokenSequence::end() {
    return tokenList.end();
}

TokenSeqConstIter TokenSequence::cBegin() {
    return tokenList.cbegin();
}

TokenSeqConstIter TokenSequence::cEnd() {
    return tokenList.cend();
}

NumericLiteralParser::NumericLiteralParser(const std::string &str, SourceLocation Loc) {
}