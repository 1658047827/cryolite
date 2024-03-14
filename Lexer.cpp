#include "Lexer.h"
#include "Diagnostic.h"
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>

Cursor::Cursor(std::string *filename, std::vector<char> &buf)
    : loc{filename, 1, 1}, ptr(buf.data()) {}

SourceLocation Cursor::getLoc() const { return loc; }

const char *Cursor::getPtr() const { return ptr; }

Cursor &Cursor::operator++() {
    if (*ptr == '\n') {
        loc.line += 1;
        loc.column = 1;
    } else {
        loc.column += 1;
    }
    ++ptr;
    return *this;
}

Cursor Cursor::operator++(int) {
    Cursor tmp = *this;
    ++(*this);
    return tmp;
}

const char &Cursor::operator*() { return *ptr; }

Cursor Cursor::operator+(int distance) const {
    Cursor result = *this;
    for (int i = 0; i < distance; ++i)
        ++result;
    return result;
}

unsigned Cursor::operator-(const Cursor &other) const {
    return ptr - other.ptr;
}

static inline bool isNumberBody(char c) {
    return std::isdigit(c) || std::isalpha(c) || c == '.';
}

static inline bool isIdentifierBody(char c) {
    return std::isdigit(c) || std::isalpha(c) || c == '_';
}

Lexer::Lexer(std::string *filename, std::vector<char> &buf, IdentifierTable &idTable)
    : buffer(buf),
      bufferCursor(filename, buf),
      identifierTable(idTable) {}

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

    // Read a character, advancing over it.
    char c = *(curCursor++);
    TokenKind kind;

    switch (c) {
    case 0:
        tok.setKind(TokenKind::TK_EOF);
        return;
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
        if (*curCursor == '>') {
            kind = TokenKind::TK_RSQB; // ':>' -> ']'
            ++curCursor;
        } else {
            kind = TokenKind::TK_COLON;
        }
        break;
    case '=':
        if (*curCursor == '=') {
            kind = TokenKind::TK_EQUALEQUAL;
            ++curCursor;
        } else {
            kind = TokenKind::TK_EQUAL;
        }
        break;
    case '.':
        c = *curCursor;
        if (std::isdigit(c)) {
            return lexNumericConstant(tok, ++curCursor);
        } else if (c == '.' && *(curCursor + 1) == '.') {
            kind = TokenKind::TK_ELLIPSIS;
            ++(++curCursor);
        } else {
            kind = TokenKind::TK_DOT;
        }
        break;
    case '&':
        c = *curCursor;
        if (c == '&') {
            kind = TokenKind::TK_AMPAMP;
            ++curCursor;
        } else if (c == '=') {
            kind = TokenKind::TK_AMPEQUAL;
            ++curCursor;
        } else {
            kind = TokenKind::TK_AMP;
        }
        break;
    case '*':
        if (*curCursor == '=') {
            kind = TokenKind::TK_STAREQUAL;
            ++curCursor;
        } else {
            kind = TokenKind::TK_STAR;
        }
        break;
    case '+':
        c = *curCursor;
        if (c == '+') {
            kind = TokenKind::TK_PLUSPLUS;
            ++curCursor;
        } else if (c == '=') {
            kind = TokenKind::TK_PLUSEQUAL;
            ++curCursor;
        } else {
            kind = TokenKind::TK_PLUS;
        }
        break;
    case '-':
        c = *curCursor;
        if (c == '-') {
            kind = TokenKind::TK_MINUSMINUS;
            ++curCursor;
        } else if (c == '>') {
            kind = TokenKind::TK_ARROW;
            ++curCursor;
        } else if (c == '=') {
            kind = TokenKind::TK_MINUSEQUAL;
            ++curCursor;
        } else {
            kind = TokenKind::TK_MINUS;
        }
        break;
    case '!':
        if (*curCursor == '=') {
            kind = TokenKind::TK_EXCLAIMEQUAL;
            ++curCursor;
        } else {
            kind = TokenKind::TK_EXCLAIM;
        }
        break;
    case '/':
        c = *curCursor;

        if (c == '/') {
            skipBCPLComment(++curCursor);
            goto lexNextToken;
        }

        if (c == '*') {
            skipBlockComment(++curCursor);
            goto lexNextToken;
        }

        if (c == '=') {
            kind = TokenKind::TK_SLASHEQUAL;
            ++curCursor;
        } else {
            kind = TokenKind::TK_SLASH;
        }
        break;
    case '%':
        c = *curCursor;
        if (c == '=') {
            kind = TokenKind::TK_PERCENTEQUAL;
            ++curCursor;
        } else if (c == '>') {
            kind = TokenKind::TK_RBRACE; // '%>' -> '}'
            ++curCursor;
        } else if (c == ':') {
            ++curCursor;
            c = *curCursor;
            if (c == '%' && *(curCursor + 1) == ':') {
                kind = TokenKind::TK_HASHHASH; // '%:%:' -> '##'
                ++(++curCursor);
            } else {
                kind = TokenKind::TK_HASH; // '%:' -> '#'
            }
        } else {
            kind = TokenKind::TK_PERCENT;
        }
        break;
    case '<':
        c = *curCursor;
        if (c == '<' && *(curCursor + 1) == '=') {
            kind = TokenKind::TK_LESSLESSEQUAL;
            ++(++curCursor);
        } else if (c == '<') {
            kind = TokenKind::TK_LESSLESS;
            ++curCursor;
        } else if (c == '=') {
            kind = TokenKind::TK_LESSEQUAL;
            ++curCursor;
        } else if (c == ':') {
            kind = TokenKind::TK_LSQB; // '<:' -> '['
            ++curCursor;
        } else if (c == '%') {
            kind = TokenKind::TK_LBRACE; // '<%' -> '{'
            ++curCursor;
        } else {
            kind = TokenKind::TK_LESS;
        }
        break;
    case '>':
        c = *curCursor;
        if (c == '=') {
            kind = TokenKind::TK_GREATEREQUAL;
            ++curCursor;
        } else if (c == '>' && *(curCursor + 1) == '=') {
            kind = TokenKind::TK_GREATERGREATEREQUAL;
            ++(++curCursor);
        } else if (c == '>') {
            kind = TokenKind::TK_GREATERGREATER;
            ++curCursor;
        } else {
            kind = TokenKind::TK_GREATER;
        }
        break;
    case '^':
        if (*curCursor == '=') {
            kind = TokenKind::TK_CARETEQUAL;
            ++curCursor;
        } else {
            kind = TokenKind::TK_CARET;
        }
        break;
    case '|':
        c = *curCursor;
        if (c == '=') {
            kind = TokenKind::TK_PIPEEQUAL;
            ++curCursor;
        } else if (c == '|') {
            kind = TokenKind::TK_PIPEPIPE;
            ++curCursor;
        } else {
            kind = TokenKind::TK_PIPE;
        }
        break;
    case '#':
        if (*curCursor == '#') {
            kind = TokenKind::TK_HASHHASH;
            ++curCursor;
        } else {
            kind = TokenKind::TK_HASH;
        }
        break;
    case '\'':
        return lexCharConstant(tok, curCursor);
    case '\"':
        return lexStringLiteral(tok, curCursor);
    case '\r':
        if (*curCursor == '\n')
            ++curCursor;
        // fall through
    case '\n':
        skipWhiteSpace(curCursor);
        goto lexNextToken;
    case ' ':
    case '\t':
    case '\v':
    case '\f':
        skipWhiteSpace(curCursor);
    skipIgnoredUnits:
        curCursor = bufferCursor;
        // If the next token is obviously a BCPL or block comment,
        // skip it efficiently too (without going through the big switch stmt).
        if (*curCursor == '/' && *(curCursor + 1) == '/') {
            skipBCPLComment(curCursor + 2);
            goto skipIgnoredUnits;
        } else if (*curCursor == '/' && *(curCursor + 1) == '*') {
            skipBlockComment(curCursor + 2);
            goto skipIgnoredUnits;
        }
        goto lexNextToken;
    case 'A' ... 'Z':
    case 'a' ... 'z':
    case '_':
        return lexIdentifier(tok, curCursor);
    case '0' ... '9':
        return lexNumericConstant(tok, curCursor);
    default:
        std::cout << c << std::endl;
    }

    // Update the location of token as well as bufferCursor.
    formTokenWithChars(tok, curCursor, kind);
}

void Lexer::formTokenWithChars(Token &tok, Cursor tokEnd, TokenKind kind) {
    unsigned tokLen = tokEnd - bufferCursor;
    tok.setLength(tokLen);
    tok.setLoc(bufferCursor.getLoc());
    tok.setKind(kind);
    bufferCursor = tokEnd;
}

// skipWhiteSpace - Efficiently skip over a series of whitespace characters.
// Update bufferCursor to point to the next non-whitespace character and return.
void Lexer::skipWhiteSpace(Cursor curCursor) {
    char c = *curCursor;
    while (std::isspace(c)) {
        c = *(++curCursor);
    }
    bufferCursor = curCursor;
}

// skipBCPLComment - We have just read the // characters. Skip
// until we find the newline character thats terminate the comment.
void Lexer::skipBCPLComment(Cursor curCursor) {
    char c = *curCursor;
    while (c != '\n' && c != 0) {
        ++curCursor;
        c = *curCursor;
    }
    if (c == '\n')
        ++curCursor;
    bufferCursor = curCursor;
}

// skipBlockComment - We have just read the /* characters. Skip
// until we find the */ characters that terminate the comment.
void Lexer::skipBlockComment(Cursor curCursor) {
    char c = *curCursor;
    char prevCh = 0;
    while (true) {
        while (c != '/' && c != 0) {
            ++curCursor;
            prevCh = c;
            c = *curCursor;
        }
        // Found slash or EOF.
        if (c == '/' && prevCh == '*') {
            ++curCursor;
            break;
        } else if (c == 0) {
            error(bufferCursor.getLoc(), "unterminated block comment");
            break;
        }
        prevCh = c;
        c = *(++curCursor);
    }
    bufferCursor = curCursor;
}

// lexNumericConstant - Lex the remainder of a integer or floating point
// constant. The first character, which is pointed by bufferCursor, has been lexed.
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
    tok.setLiteralPtr(tokStart.getPtr());
}

// lexCharConstant - Lex the remainder of a character constant, after having lexed '.
void Lexer::lexCharConstant(Token &tok, Cursor curCursor) {
    char c = *curCursor;
    if (c == '\'') {
        error(bufferCursor.getLoc(), "empty character constant");
        formTokenWithChars(tok, ++curCursor, TokenKind::TK_UNKNOWN);
        return;
    }

    do {
        // Skip escaped characters.
        if (c == '\\') {
            ++curCursor;
        } else if (c == '\n' || c == '\r' || c == 0) {
            error(bufferCursor.getLoc(), "unterminated character constant");
            formTokenWithChars(tok, curCursor, TokenKind::TK_UNKNOWN);
            return;
        }
        c = *(++curCursor);
    } while (c != '\'');

    Cursor tokStart = bufferCursor;
    formTokenWithChars(tok, ++curCursor, TokenKind::TK_CHAR_CONSTANT);
    tok.setLiteralPtr(tokStart.getPtr());
}

// lexStringLiteral - Lex the remainder of a string literal, after having lexed ".
void Lexer::lexStringLiteral(Token &tok, Cursor curCursor) {
    char c = *curCursor;
    while (c != '"') {
        // Skip escaped characters.
        if (c == '\\') {
            ++curCursor;
        } else if (c == '\n' || c == '\r' || c == 0) {
            error(bufferCursor.getLoc(), "unterminated string literal");
            formTokenWithChars(tok, curCursor, TokenKind::TK_UNKNOWN);
            return;
        }
        c = *(++curCursor);
    }

    Cursor tokStart = bufferCursor;
    formTokenWithChars(tok, ++curCursor, TokenKind::TK_STRING_LITERAL);
    tok.setLiteralPtr(tokStart.getPtr());
}

void Lexer::lexIdentifier(Token &tok, Cursor curCursor) {
    char c = *curCursor;
    while (isIdentifierBody(c)) {
        c = *(++curCursor);
    }
    Cursor IdStart = bufferCursor;
    formTokenWithChars(tok, curCursor, TokenKind::TK_IDENTIFIER);

    // Fill tok.ptr with IdentifierInfo* retrieved from the identifier table.
    IdentifierInfo *info = lookUpIdentifierInfo(tok, IdStart.getPtr());
}

// lookUpIdentifierInfo - Given a TokenKind::TK_IDENTIFIER token, look up the
// identifier information for the token and install it into the token.
IdentifierInfo *Lexer::lookUpIdentifierInfo(Token &tok, const char *bufStart) {
    assert(tok.is(TokenKind::TK_IDENTIFIER) && "not an identifier");
    assert(tok.getIdentifierInfo() == nullptr && "exist IdentifierInfo");

    IdentifierInfo *info = identifierTable.get(bufStart, bufStart + tok.getLength());
    tok.setIdentifierInfo(info);
    return info;
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