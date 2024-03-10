#include "Lexer.h"
#include "Diagnostic.h"
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>

static const std::unordered_map<std::string, TokenKind> keyword2TokenKind{
#define KEYWORD(X, Y) {Y, TK_##X},
#include "TokenKind.def"
};

static inline bool isNumberBody(char c) {
    return std::isdigit(c) || std::isalpha(c) || c == '.';
}

void Lexer::lex(TokenSequence &ts) {
    char c = curChar();
    while (c != 0) {
        switch (c) {
        case '?':
            ts.emplaceBack(TK_QUESTION, curLoc);
            break;
        case '(':
            ts.emplaceBack(TK_LPAR, curLoc);
            break;
        case ')':
            ts.emplaceBack(TK_RPAR, curLoc);
            break;
        case '[':
            ts.emplaceBack(TK_LSQB, curLoc);
            break;
        case ']':
            ts.emplaceBack(TK_RSQB, curLoc);
            break;
        case '{':
            ts.emplaceBack(TK_LBRACE, curLoc);
            break;
        case '}':
            ts.emplaceBack(TK_RBRACE, curLoc);
            break;
        case '~':
            ts.emplaceBack(TK_TILDE, curLoc);
            break;
        case ';':
            ts.emplaceBack(TK_SEMI, curLoc);
            break;
        case ',':
            ts.emplaceBack(TK_COMMA, curLoc);
            break;
        case ':':
            ts.emplaceBack(TK_COLON, curLoc);
            break;
        case '=': {
            if (nextIs('=')) {
                ts.emplaceBack(TK_EQUALEQUAL, curLoc);
                next(); // Consume the '='.
            } else {
                ts.emplaceBack(TK_EQUAL, curLoc);
            }
        } break;
        case '.': {
            char peekc = lookAhead();
            if (std::isdigit(peekc)) {
                Token tk = lexNumericConstant();
                ts.pushBack(tk);
            } else if (peekc == '.' && lookAhead(2) == '.') {
                ts.emplaceBack(TK_ELLIPSIS, curLoc);
                next(); // Consume the two dots.
                next();
            } else {
                ts.emplaceBack(TK_DOT, curLoc);
            }
        } break;
        case '&': {
            char peekc = lookAhead();
            if (peekc == '&') {
                ts.emplaceBack(TK_AMPAMP, curLoc);
                next(); // Consume the '&'.
            } else if (peekc == '=') {
                ts.emplaceBack(TK_AMPEQUAL, curLoc);
                next(); // Consume thr '='.
            } else {
                ts.emplaceBack(TK_AMP, curLoc);
            }
        } break;
        case '*': {
            if (lookAhead() == '=') {
                ts.emplaceBack(TK_STAREQUAL, curLoc);
                next(); // Consume the '='.
            } else {
                ts.emplaceBack(TK_STAR, curLoc);
            }
        } break;
        case '+': {
            char peekc = lookAhead();
            if (peekc == '+') {
                ts.emplaceBack(TK_PLUSPLUS, curLoc);
                next(); // Consume the '+'.
            } else if (peekc == '=') {
                ts.emplaceBack(TK_PLUSEQUAL, curLoc);
                next(); // Consume the '='.
            } else {
                ts.emplaceBack(TK_PLUS, curLoc);
            }
        } break;
        case '-': {
            char peekc = lookAhead();
            if (peekc == '-') {
                ts.emplaceBack(TK_MINUSMINUS, curLoc);
                next(); // Consume the '-'.
            } else if (peekc == '>') {
                ts.emplaceBack(TK_ARROW, curLoc);
                next(); // Consume the '>'.
            } else if (peekc == '=') {
                ts.emplaceBack(TK_MINUSEQUAL, curLoc);
                next(); // Consume the '='.
            } else {
                ts.emplaceBack(TK_MINUS, curLoc);
            }
        } break;
        case '!': {
            if (nextIs('=')) {
                ts.emplaceBack(TK_EXCLAIMEQUAL, curLoc);
                next(); // Consume the '='.
            } else {
                ts.emplaceBack(TK_EXCLAIM, curLoc);
            }
        } break;
        case '/': {
            if (nextIs('/') || nextIs('*')) {
                skipComment();
            } else if (nextIs('=')) {
                ts.emplaceBack(TK_SLASHEQUAL, curLoc);
                next(); // Consume the '='.
            } else {
                ts.emplaceBack(TK_SLASH, curLoc);
            }
        } break;
        case '%': {
            if (lookAhead() == '=') {
                ts.emplaceBack(TK_PERCENTEQUAL, curLoc);
                next(); // Consume the '='.
            } else {
                ts.emplaceBack(TK_PERCENT, curLoc);
            }
        } break;
        case '<': {
            char peekc = lookAhead();
            if (peekc == '<' && lookAhead(2) == '=') {
                ts.emplaceBack(TK_LESSLESSEQUAL, curLoc);
                next(); // Consume the "<=".
                next();
            } else if (peekc == '<') {
                ts.emplaceBack(TK_LESSLESS, curLoc);
                next(); // Consume the '<'.
            } else if (peekc == '=') {
                ts.emplaceBack(TK_LESSEQUAL, curLoc);
                next(); // Consume the '='.
            } else {
                ts.emplaceBack(TK_LESS, curLoc);
            }
        } break;
        case '>': {
            char peekc = lookAhead();
            if (peekc == '=') {
                ts.emplaceBack(TK_GREATEREQUAL, curLoc);
                next(); // Consume the '='.
            } else if (peekc == '>' && lookAhead(2) == '=') {
                ts.emplaceBack(TK_GREATERGREATEREQUAL, curLoc);
                next(); // Consume the ">=".
                next();
            } else if (peekc == '>') {
                ts.emplaceBack(TK_GREATERGREATER, curLoc);
                next(); // Consume the '>'.
            } else {
                ts.emplaceBack(TK_GREATER, curLoc);
            }
        } break;
        case '^': {
            if (lookAhead() == '=') {
                ts.emplaceBack(TK_CARETEQUAL, curLoc);
                next(); // Consume the '='.
            } else {
                ts.emplaceBack(TK_CARET, curLoc);
            }
        } break;
        case '|': {
            char peekc = lookAhead();
            if (peekc == '=') {
                ts.emplaceBack(TK_PIPEEQUAL, curLoc);
                next(); // Consume the '='.
            } else if (peekc == '|') {
                ts.emplaceBack(TK_PIPEPIPE, curLoc);
                next(); // Consume the '|'.
            } else {
                ts.emplaceBack(TK_PIPE, curLoc);
            }
        } break;
        case '#': {
            if (lookAhead() == '#') {
                ts.emplaceBack(TK_HASHHASH, curLoc);
                next(); // Consume the '#'.
            } else {
                ts.emplaceBack(TK_HASH, curLoc);
            }
        } break;
        case '\'': {
            Token tk = lexCharConstant();
            ts.pushBack(tk);
        } break;
        case '\"': {
            Token tk = lexStringLiteral();
            ts.pushBack(tk);
        } break;
        case ' ':
        case '\t':
        case '\v':
        case '\n':
        case '\f':
            break; // Skip the whitespace.
        case 'A' ... 'Z':
        case 'a' ... 'z':
        case '_': {
            Token tk = lexIdentifier();
            ts.pushBack(tk);
        } break;
        case '0' ... '9': {
            Token tk = lexNumericConstant();
            ts.pushBack(tk);
        } break;
        default:
            std::cout << c << std::endl;
        }
        c = next();
    }

    ts.emplaceBack(TK_EOF, curLoc);
}

char Lexer::curChar() {
    if (p < buffer.size())
        return buffer[p];
    else
        return 0;
}

char Lexer::next() {
    char c = curChar();
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

char Lexer::lookAhead(std::size_t n) {
    if (p + n >= buffer.size())
        return 0;
    return buffer[p + n];
}

bool Lexer::nextIs(char c) {
    return lookAhead(1) == c;
}

bool Lexer::tryNext(char c) {
    if (lookAhead() == c) {
        next();
        return true;
    } else {
        return false;
    }
}

void Lexer::skipComment() {
    char c = '/';
    SourceLocation loc = curLoc;
    if (tryNext('/')) {
        while (!nextIs(0) && c != '\n') {
            c = next();
        }
    } else if (tryNext('*')) {
        while (!nextIs(0)) {
            c = next();
            if (c == '*' && nextIs('/')) {
                next();
                return;
            }
        }
        error(loc, "unterminated block comment");
    }
}

Token Lexer::lexNumericConstant() {
    std::size_t begin = p;
    SourceLocation loc = curLoc;
    char c = lookAhead();
    char prevCh = 0;
    while (isNumberBody(c)) {
        next();
        prevCh = c;
        c = lookAhead();
    }

    // If we fell out, check for a sign, due to 1e+12. If we have one, continue.
    if ((c == '-' || c == '+') && (prevCh == 'E' || prevCh == 'e')) {
        next();
        lexNumericConstant();
    }

    // If we have a hex FP constant, continue.
    if ((c == '-' || c == '+') && (prevCh == 'P' || prevCh == 'p')) {
        next();
        lexNumericConstant();
    }

    std::string tkStr(buffer.begin() + begin, buffer.begin() + p + 1);
    return Token(TK_NUMERIC_CONSTANT, loc, tkStr);
}

Token Lexer::lexCharConstant() {
    std::size_t begin = p;
    SourceLocation loc = curLoc;
    if (tryNext('\'')) {
        error(loc, "empty character constant");
        return Token(TK_UNKNOWN, loc);
    }
    char c = next();
    while (c != '\'' && c != '\n' && c != 0) {
        // Skip escaped characters.
        if (c == '\\') {
            next();
        }
        c = next();
    }
    if (c != '\'') {
        error(loc, "unterminated character constant");
        return Token(TK_UNKNOWN, loc);
    }
    std::string tkStr(buffer.begin() + begin, buffer.begin() + p + 1);
    return Token(TK_CHAR_CONSTANT, loc, tkStr);
}

Token Lexer::lexStringLiteral() {
    std::size_t begin = p;
    SourceLocation loc = curLoc;
    char c = next();
    while (c != '\"') {
        // Skip escaped characters.
        if (c == '\\') {
            next();
        } else if (c == '\n' || c == 0) {
            error(loc, "unterminated string literal");
            return Token(TK_UNKNOWN, loc);
        }
        c = next();
    }
    std::string tkStr(buffer.begin() + begin, buffer.begin() + p + 1);
    return Token(TK_STRING_LITERAL, loc, tkStr);
}

Token Lexer::lexIdentifier() {
    std::size_t begin = p;
    SourceLocation loc = curLoc;
    char c = lookAhead();
    while (c != 0) {
        if (!std::isalpha(c) && !std::isdigit(c) && c != '_')
            break;
        next();
        c = lookAhead();
    }
    std::string tkStr(buffer.begin() + begin, buffer.begin() + p + 1);
    auto iter = keyword2TokenKind.find(tkStr);
    if (iter != keyword2TokenKind.cend()) {
        return Token(iter->second, loc);
    }
    return Token(TK_IDENTIFIER, loc, tkStr);
}

int readFile(std::string &filename, std::vector<char> &buffer) {
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