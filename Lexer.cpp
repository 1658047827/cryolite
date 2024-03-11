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

void Lexer::lex(Token &tok) {
    tok.clear();
lexNextToken:
    // curCursor - Cache bufferCursor in an automatic variable.
    Cursor curCursor = bufferCursor;

    // Skip horizontal whitespace.
    if (*curCursor == ' ' || *curCursor == '\t') {
        ++curCursor;
        while (*curCursor == ' ' || *curCursor == '\t')
            ++curCursor;
        bufferCursor = curCursor;
    }

    char c = getAndAdvance(curCursor);
    TokenKind kind;

    switch (c) {
    case '?':
        kind = TokenKind::TK_QUESTION;
        break;
    case '(':
        kind = TokenKind::TK_LPAR;
        break;
    case ')':
        kind = TokenKind::TK_RPAR;
        break;
    case '[':
        kind = TokenKind::TK_LSQB;
        break;
    case ']':
        kind = TokenKind::TK_RSQB;
        break;
    case '{':
        kind = TokenKind::TK_LBRACE;
        break;
    case '}':
        kind = TokenKind::TK_RBRACE;
        break;
    case '~':
        kind = TokenKind::TK_TILDE;
        break;
    case ';':
        kind = TokenKind::TK_SEMI;
        break;
    case ',':
        kind = TokenKind::TK_COMMA;
        break;
    case ':':
        c = lookAhead();
        if (c == '>') {
            kind = TokenKind::TK_RSQB; // ':>' -> ']'
            next();
        } else {
            kind = TokenKind::TK_COLON;
        }
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

    ts.emplaceBack(TK_EOF, curLoc);
}

char Lexer::lookAhead(std::size_t n) {
    if (bufferPtr + n >= buffer.size())
        return 0;
    return buffer[bufferPtr + n];
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

void Lexer::lexNumericConstant(Token &tok, Cursor curCursor) {
    char c = *curCursor;
    char prevCh = 0;
    while (isNumberBody(c)) {
        ++curCursor;
        prevCh = c;
        c = *curCursor;
    }

    // If we fell out, check for a sign, due to 1e+12. If we have one, continue.
    if ((c == '-' || c == '+') && (prevCh == 'E' || prevCh == 'e')) {
        return lexNumericConstant(tok, ++curCursor);
    }

    // If we have a hex FP constant, continue.
    if ((c == '-' || c == '+') && (prevCh == 'P' || prevCh == 'p')) {
        return lexNumericConstant(tok, ++curCursor);
    }

    Cursor tokStart = bufferCursor;
    formTokenWithChars(tok, curCursor, TokenKind::TK_NUMERIC_CONSTANT);
    tok.setLiteralPtr(tokStart.base());
}

Token Lexer::lexCharConstant() {
    std::size_t begin = bufferPtr;
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
    std::string tkStr(buffer.begin() + begin, buffer.begin() + bufferPtr + 1);
    return Token(TK_CHAR_CONSTANT, loc, tkStr);
}

Token Lexer::lexStringLiteral() {
    std::size_t begin = bufferPtr;
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
    std::string tkStr(buffer.begin() + begin, buffer.begin() + bufferPtr + 1);
    return Token(TK_STRING_LITERAL, loc, tkStr);
}

Token Lexer::lexIdentifier() {
    std::size_t begin = bufferPtr;
    SourceLocation loc = curLoc;
    char c = lookAhead();
    while (c != 0) {
        if (!std::isalpha(c) && !std::isdigit(c) && c != '_')
            break;
        next();
        c = lookAhead();
    }
    std::string tkStr(buffer.begin() + begin, buffer.begin() + bufferPtr + 1);
    auto iter = keyword2TokenKind.find(tkStr);
    if (iter != keyword2TokenKind.cend()) {
        return Token(iter->second, loc);
    }
    return Token(TK_IDENTIFIER, loc, tkStr);
}

std::vector<char> readFile(std::string &filename) {
    std::ifstream input(filename);
    if (!input.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return {0};
    }
    std::vector<char> buffer(std::istreambuf_iterator<char>(input), {});
    buffer.push_back(0);
    return std::move(buffer);
}