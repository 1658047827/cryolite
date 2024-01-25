#include <algorithm>
#include <iostream>
#include <unordered_map>

#include "token.h"

static const std::unordered_map<TokenKind, std::string> tokenKind2Str{
    //     {TokenKind::TK_UNKNOWN, ""},
    //     {TokenKind::TK_EOF,     ""},
    //     {TokenKind::TK_EOM,     ""},

    //     // Identifier
    //     {TokenKind::TK_IDENTIFIER, ""},

    //     // Constants
    //     {TK_NUMERIC_CONSTANT, ""},
    //     {TK_CHAR_CONSTANT,    ""},

    //     // String Literal
    //     {TK_STRING_LITERAL, ""},

    //     // Punctuators
    //     TK_LPAR,                // (
    //     TK_RPAR,                // )
    //     TK_LSQB,                // [
    //     TK_RSQB,                // ]
    //     TK_LBRACE,              // {
    //     TK_RBRACE,              // }
    //     TK_DOT,                 // .
    //     TK_ellipsis,            // ...
    //     TK_AMP,                 // &
    //     TK_AMPAMP,              // &&
    //     TK_AMPEQUAL,            // &=
    //     TK_STAR,                // *
    //     TK_STAREQUAL,           // *=
    //     TK_PLUS,                // +
    //     TK_PLUSPLUS,            // ++
    //     TK_PLUSEQUAL,           // +=
    //     TK_MINUS,               // -
    //     TK_ARROW,               // ->
    //     TK_MINUSMINUS,          // --
    //     TK_MINUSEQUAL,          // -=
    //     TK_TILDE,               // ~
    //     TK_EXCLAIM,             // !
    //     TK_EXCLAIMEQUAL,        // !=
    //     TK_SLASH,               // /
    //     TK_SLASHEQUAL,          // /=
    //     TK_PERCENT,             // %
    //     TK_PERCENTEQUAL,        // %=
    //     TK_LESS,                // <
    //     TK_LESSLESS,            // <<
    //     TK_LESSEQUAL,           // <=
    //     TK_LESSLESSEQUAL,       // <<=
    //     TK_GREATER,             // >
    //     TK_GREATERGREATER,      // >>
    //     TK_GREATEREQUAL,        // >=
    //     TK_GREATERGREATEREQUAL, // >>=
    //     TK_CARET,               // ^
    //     TK_CARETEQUAL,          // ^=
    //     TK_PIPE,                // |
    //     TK_PIPEPIPE,            // ||
    //     TK_PIPEEQUAL,           // |=
    //     TK_QUESTION,            // ?
    //     TK_COLON,               // :
    {TK_SEMI, ";"},
    {TK_EQUAL, "="},
    //     TK_EQUALEQUAL,          // ==
    //     TK_COMMA,               // ,
    //     TK_HASH,                // #
    //     TK_HASHHASH,            // ##

    //     // C Keywords
    //     TK_BREAK,
    {TK_CASE, "case"},
    {TK_CHAR, "char"},
    //     TK_CONST,
    //     TK_CONTINUE,
    //     TK_DEFAULT,
    //     TK_DO,
    //     TK_DOUBLE,
    //     TK_ELSE,
    //     TK_ENUM,
    //     TK_EXTERN,
    //     TK_FLOAT,
    //     TK_FOR,
    //     TK_GOTO,
    //     TK_IF,
    //     TK_INLINE,
    //     TK_INT,
    //     TK_LONG,
    //     TK_RETURN,
    //     TK_SHORT,
    //     TK_SIGNED,
    //     TK_SIZEOF,
    //     TK_STATIC,
    //     TK_STRUCT,
    //     TK_SWITCH,
    //     TK_TYPEDEF,
    //     TK_UNION,
    //     TK_UNSIGNED,
    //     TK_VOID,
    //     TK_WHILE,

    //     // For preprocessor
    //     PP_IF,
    //     PP_IFDEF,
    //     PP_IFNDEF,
    //     PP_ELIF,
    //     PP_ELSE,
    //     PP_ENDIF,
    //     PP_INCLUDE,
    //     PP_DEFINE,
    //     PP_UNDEF,
    //     PP_LINE,
    //     PP_ERROR,
    //     PP_PRAGMA,
    //     PP_NONE,
    //     PP_EMPTY,
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