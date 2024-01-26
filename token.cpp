#include <algorithm>
#include <iostream>
#include <unordered_map>

#include "token.h"

static const std::unordered_map<TokenKind, std::string> tokenKind2Str{
    // Punctuators
    {TK_LPAR, "("},
    {TK_RPAR, ")"},
    {TK_LSQB, "["},
    {TK_RSQB, "]"},
    {TK_LBRACE, "{"},
    {TK_RBRACE, "}"},
    {TK_DOT, "."},
    {TK_ELLIPSIS, "..."},
    {TK_AMP, "&"},
    {TK_AMPAMP, "&&"},
    {TK_AMPEQUAL, "&="},
    {TK_STAR, "*"},
    {TK_STAREQUAL, "*="},
    {TK_PLUS, "+"},
    {TK_PLUSPLUS, "++"},
    {TK_PLUSEQUAL, "+="},
    {TK_MINUS, "-"},
    {TK_ARROW, "->"},
    {TK_MINUSMINUS, "--"},
    {TK_MINUSEQUAL, "-="},
    {TK_TILDE, "~"},
    {TK_EXCLAIM, "!"},
    {TK_EXCLAIMEQUAL, "!="},
    {TK_SLASH, "/"},
    {TK_SLASHEQUAL, "/="},
    {TK_PERCENT, "%"},
    {TK_PERCENTEQUAL, "%="},
    {TK_LESS, "<"},
    {TK_LESSLESS, "<<"},
    {TK_LESSEQUAL, "<="},
    {TK_LESSLESSEQUAL, "<<="},
    {TK_GREATER, ">"},
    {TK_GREATERGREATER, ">>"},
    {TK_GREATEREQUAL, ">="},
    {TK_GREATERGREATEREQUAL, ">>="},
    {TK_CARET, "^"},
    {TK_CARETEQUAL, "^="},
    {TK_PIPE, "|"},
    {TK_PIPEPIPE, "||"},
    {TK_PIPEEQUAL, "|="},
    {TK_QUESTION, "?"},
    {TK_COLON, ":"},
    {TK_SEMI, ";"},
    {TK_EQUAL, "="},
    {TK_EQUALEQUAL, "=="},
    {TK_COMMA, ","},
    {TK_HASH, "#"},
    {TK_HASHHASH, "##"},

    // C Keywords
    {TK_BREAK, "break"},
    {TK_CASE, "case"},
    {TK_CHAR, "char"},
    {TK_CONST, "const"},
    {TK_CONTINUE, "continue"},
    {TK_DEFAULT, "default"},
    {TK_DO, "do"},
    {TK_DOUBLE, "double"},
    {TK_ELSE, "else"},
    {TK_ENUM, "enum"},
    {TK_EXTERN, "extern"},
    {TK_FLOAT, "float"},
    {TK_FOR, "for"},
    {TK_GOTO, "goto"},
    {TK_IF, "if"},
    {TK_INLINE, "inline"},
    {TK_INT, "int"},
    {TK_LONG, "long"},
    {TK_RETURN, "return"},
    {TK_SHORT, "short"},
    {TK_SIGNED, "signed"},
    {TK_SIZEOF, "sizeof"},
    {TK_STATIC, "static"},
    {TK_STRUCT, "struct"},
    {TK_SWITCH, "switch"},
    {TK_TYPEDEF, "typedef"},
    {TK_UNION, "union"},
    {TK_UNSIGNED, "unsigned"},
    {TK_VOID, "void"},
    {TK_WHILE, "while"},

    // For preprocessor
    {PP_IF, "if"},
    {PP_IFDEF, "ifdef"},
    {PP_IFNDEF, "ifndef"},
    {PP_ELIF, "elif"},
    {PP_ELSE, "else"},
    {PP_ENDIF, "endif"},
    {PP_INCLUDE, "include"},
    {PP_DEFINE, "define"},
    {PP_UNDEF, "undef"},
    {PP_LINE, "line"},
    {PP_ERROR, "error"},
    {PP_PRAGMA, "pragma"},
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