#include "TokenKind.h"

std::string_view getTokenSpelling(TokenKind kind) {
    switch (kind) {
#define PUNCTUATOR(X, Y)    \
    case TokenKind::TK_##X: \
        return Y;
#define KEYWORD(X, Y)       \
    case TokenKind::TK_##X: \
        return Y;
#include "TokenKind.def"
    default:
        break;
    }
    return std::string_view();
}