#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>

#include "feedback.h"
#include "lexer.h"

static const std::unordered_map<std::string, TokenKind> keyword2TokenKind{
    // TK_BREAK,
    // TK_CASE,
    {"char", TK_CHAR},
    // TK_CONST,
    // TK_CONTINUE,
    // TK_DEFAULT,
    // TK_DO,
    // TK_DOUBLE,
    // TK_ELSE,
    // TK_ENUM,
    // TK_EXTERN,
    // TK_FLOAT,
    // TK_FOR,
    // TK_GOTO,
    // TK_IF,
    // TK_INLINE,
    // TK_INT,
    // TK_LONG,
    // TK_RETURN,
    // TK_SHORT,
    // TK_SIGNED,
    // TK_SIZEOF,
    // TK_STATIC,
    // TK_STRUCT,
    // TK_SWITCH,
    // TK_TYPEDEF,
    // TK_UNION,
    // TK_UNSIGNED,
    // TK_VOID,
    // TK_WHILE,
};

void Lexer::Lex(TokenSequence &ts) {
    char c = CurChar();
    while (c != 0) {
        switch (c) {
        case '?':
            ts.EmplaceBack(TokenKind::TK_QUESTION, curLoc);
            break;
        case '(':
            ts.EmplaceBack(TokenKind::TK_LPAR, curLoc);
            break;
        case ')':
            ts.EmplaceBack(TokenKind::TK_RPAR, curLoc);
            break;
        case '[':
            ts.EmplaceBack(TokenKind::TK_LSQB, curLoc);
            break;
        case ']':
            ts.EmplaceBack(TokenKind::TK_RSQB, curLoc);
            break;
        case '{':
            ts.EmplaceBack(TokenKind::TK_LBRACE, curLoc);
            break;
        case '}':
            ts.EmplaceBack(TokenKind::TK_RBRACE, curLoc);
            break;
        case '~':
            ts.EmplaceBack(TokenKind::TK_TILDE, curLoc);
            break;
        case ';':
            ts.EmplaceBack(TokenKind::TK_SEMI, curLoc);
            break;
        case ',':
            ts.EmplaceBack(TokenKind::TK_COMMA, curLoc);
            break;
        case ':':
            ts.EmplaceBack(TokenKind::TK_COLON, curLoc);
            break;
        case '=': {
            if (Try('=')) {
                ts.EmplaceBack(TokenKind::TK_EQUALEQUAL, curLoc);
            } else {
                ts.EmplaceBack(TokenKind::TK_EQUAL, curLoc);
            }
        } break;
        case '/': {
            if (NextIs('/') || NextIs('*')) {
                SkipComment();
            } else if (Try('=')) {
                ts.EmplaceBack(TokenKind::TK_SLASHEQUAL, curLoc);
            } else {
                ts.EmplaceBack(TokenKind::TK_SLASH, curLoc);
            }
        } break;
        case '\\':

            break;
        case '\'': {
            Token tk = LexCharConstant();
            ts.PushBack(tk);
        } break;
        case '\"': {
            Token tk = LexStringLiteral();
            ts.PushBack(tk);
        } break;
        case ' ':
        case '\t':
        case '\n':
            break;
        case 'A' ... 'Z':
        case 'a' ... 'z':
        case '_': {
            Token tk = LexIdentifier();
            ts.PushBack(tk);
        } break;
        default:
            std::cout << c << std::endl;
        }
        c = Next();
    }

    ts.EmplaceBack(TokenKind::TK_EOF, curLoc);
}

char Lexer::CurChar() {
    if (p < buffer.size())
        return buffer[p];
    else
        return 0;
}

char Lexer::Next() {
    char c = CurChar();
    if (c == 0)
        return 0;
    if (c == '\n') {
        ++curLoc.line;
        curLoc.column = 1;
    } else {
        ++curLoc.column;
    }
    ++p;
    if (p >= buffer.size())
        return 0;
    return buffer[p];
}

char Lexer::LookAhead(size_t n = 1) {
    if (p + n >= buffer.size())
        return 0;
    return buffer[p + n];
}

bool Lexer::NextIs(char c) {
    return LookAhead(1) == c;
}

bool Lexer::Try(char c) {
    if (LookAhead(1) == c) {
        Next();
        return true;
    } else {
        return false;
    }
}

void Lexer::SkipComment() {
    char c = '/';
    SourceLocation loc = curLoc;
    if (Try('/')) {
        while (!NextIs(0) && c != '\n') {
            c = Next();
        }
    } else if (Try('*')) {
        while (!NextIs(0)) {
            c = Next();
            if (c == '*' && NextIs('/')) {
                Next();
                return;
            }
        }
        Error(loc, "unterminated block comment");
    }
}

Token Lexer::LexCharConstant() {
    size_t begin = p;
    SourceLocation loc = curLoc;
    if (Try('\'')) {
        Error(loc, "empty character constant");
        return Token(TokenKind::TK_UNKNOWN, loc);
    }
    char c = Next();
    while (c != '\'' && c != '\n' && c != 0) {
        if (c == '\\') {
            Next();
        }
        c = Next();
    }
    if (c != '\'') {
        Error(loc, "unterminated character constant");
        return Token(TokenKind::TK_UNKNOWN, loc);
    }
    std::string tkStr(buffer.begin() + begin, buffer.begin() + p + 1);
    return Token(TokenKind::TK_CHAR_CONSTANT, loc, tkStr);
}

Token Lexer::LexStringLiteral() {
}

Token Lexer::LexIdentifier() {
    size_t begin = p;
    SourceLocation loc = curLoc;
    char c = LookAhead();
    while (c != 0) {
        if (!std::isalpha(c) && !std::isdigit(c) && c != '_')
            break;
        Next();
        c = LookAhead();
    }
    std::string tkStr(buffer.begin() + begin, buffer.begin() + p + 1);
    auto iter = keyword2TokenKind.find(tkStr);
    if (iter != keyword2TokenKind.cend()) {
        return Token(iter->second, loc);
    }
    return Token(TokenKind::TK_IDENTIFIER, loc, tkStr);
}

int ReadFile(std::string &filename, std::vector<char> &buffer) {
    std::ifstream infile(filename);
    if (!infile.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return 1;
    }

    infile.seekg(0, std::ios::end);
    std::streamsize fileSize = infile.tellg();
    infile.seekg(0, std::ios::beg);

    buffer.resize(fileSize);
    infile.read(buffer.data(), fileSize);
    return 0;
}