#ifndef _CRYOLITE_TOKEN_KIND_H_
#define _CRYOLITE_TOKEN_KIND_H_

#include <string_view>

enum TokenKind {
#define TOK(X) TK_##X,
#include "TokenKind.def"
    NUM_TOKENS
};

std::string_view getTokenSpelling(TokenKind kind);

#endif