#ifndef _CRYOLITE_TOKEN_H_
#define _CRYOLITE_TOKEN_H_

typedef enum TokenKind {
    TK_UNKNOWN, // Not a token.
    TK_EOF,     // End of file.

    TK_IDENTIFIER, // abcde123

    TK_NUMERIC_CONSTANT, // 0x123
    TK_CHAR_CONSTANT,    // 'a'
    TK_STRING_LITERAL,   // "foo"

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

    TK_AUTO,     // auto
    TK_BOOL,     // _Bool
    TK_BREAK,    // break
    TK_CASE,     // case
    TK_CHAR,     // char
    TK_CONST,    // const
    TK_CONTINUE, // continue
    TK_DEFAULT,  // default
    TK_DO,       // do
    TK_DOUBLE,   // double
    TK_ELSE,     // else
    TK_ENUM,     // enum
    TK_EXTERN,   // extern
    TK_FLOAT,    // float
    TK_FOR,      // for
    TK_GOTO,     // goto
    TK_IF,       // if
    TK_INLINE,   // inline
    TK_INT,      // int
    TK_LONG,     // long
    TK_REGISTER, // register
    TK_RESTRICT, // restrict
    TK_RETURN,   // return
    TK_SHORT,    // short
    TK_SIGNED,   // signed
    TK_SIZEOF,   // sizeof
    TK_STATIC,   // static
    TK_STRUCT,   // struct
    TK_SWITCH,   // switch
    TK_TYPEDEF,  // typedef
    TK_UNION,    // union
    TK_UNSIGNED, // unsigned
    TK_VOID,     // void
    TK_VOLATILE, // volatile
    TK_WHILE,    // while

    NUM_TOKENS
} TokenKind;

typedef struct Token {
    TokenKind kind;
} Token;

#endif