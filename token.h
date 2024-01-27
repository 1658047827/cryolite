#ifndef _CRYOLITH_TOKEN_H_
#define _CRYOLITH_TOKEN_H_

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
    TK_UNKNOWN, // Not a token.
    TK_EOF,     // End of file.
    TK_EOM,     // End of macro (end of line inside a macro).

    // Identifier
    TK_IDENTIFIER, // abcde123

    // Constants
    TK_NUMERIC_CONSTANT, // 0x123   3.14
    TK_CHAR_CONSTANT,    // 'a'

    // String Literal
    TK_STRING_LITERAL, // "foo"

    // Punctuators
    TK_LPAR,                // (
    TK_RPAR,                // )
    TK_LSQB,                // [
    TK_RSQB,                // ]
    TK_LBRACE,              // {
    TK_RBRACE,              // }
    TK_DOT,                 // .
    TK_ELLIPSIS,            // ...
    TK_AMP,                 // &
    TK_AMPAMP,              // &&
    TK_AMPEQUAL,            // &=
    TK_STAR,                // *
    TK_STAREQUAL,           // *=
    TK_PLUS,                // +
    TK_PLUSPLUS,            // ++
    TK_PLUSEQUAL,           // +=
    TK_MINUS,               // -
    TK_ARROW,               // ->
    TK_MINUSMINUS,          // --
    TK_MINUSEQUAL,          // -=
    TK_TILDE,               // ~
    TK_EXCLAIM,             // !
    TK_EXCLAIMEQUAL,        // !=
    TK_SLASH,               // /
    TK_SLASHEQUAL,          // /=
    TK_PERCENT,             // %
    TK_PERCENTEQUAL,        // %=
    TK_LESS,                // <
    TK_LESSLESS,            // <<
    TK_LESSEQUAL,           // <=
    TK_LESSLESSEQUAL,       // <<=
    TK_GREATER,             // >
    TK_GREATERGREATER,      // >>
    TK_GREATEREQUAL,        // >=
    TK_GREATERGREATEREQUAL, // >>=
    TK_CARET,               // ^
    TK_CARETEQUAL,          // ^=
    TK_PIPE,                // |
    TK_PIPEPIPE,            // ||
    TK_PIPEEQUAL,           // |=
    TK_QUESTION,            // ?
    TK_COLON,               // :
    TK_SEMI,                // ;
    TK_EQUAL,               // =
    TK_EQUALEQUAL,          // ==
    TK_COMMA,               // ,
    TK_HASH,                // #
    TK_HASHHASH,            // ##

    // C Keywords
    TK_AUTO,
    TK_BREAK,
    TK_CASE,
    TK_CHAR,
    TK_CONST,
    TK_CONTINUE,
    TK_DEFAULT,
    TK_DO,
    TK_DOUBLE,
    TK_ELSE,
    TK_ENUM,
    TK_EXTERN,
    TK_FLOAT,
    TK_FOR,
    TK_GOTO,
    TK_IF,
    TK_INLINE,
    TK_INT,
    TK_LONG,
    TK_REGISTER,
    TK_RESTRICT,
    TK_RETURN,
    TK_SHORT,
    TK_SIGNED,
    TK_SIZEOF,
    TK_STATIC,
    TK_STRUCT,
    TK_SWITCH,
    TK_TYPEDEF,
    TK_UNION,
    TK_UNSIGNED,
    TK_VOID,
    TK_VOLATILE,
    TK_WHILE,

    // For preprocessor
    PP_IF,
    PP_IFDEF,
    PP_IFNDEF,
    PP_ELIF,
    PP_ELSE,
    PP_ENDIF,
    PP_INCLUDE,
    PP_DEFINE,
    PP_UNDEF,
    PP_LINE,
    PP_ERROR,
    PP_PRAGMA,
    PP_NONE,
    PP_EMPTY,
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