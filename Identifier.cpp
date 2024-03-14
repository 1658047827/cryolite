#include "Identifier.h"

IdentifierInfo::IdentifierInfo()
    : tokKind(TokenKind::TK_IDENTIFIER),
      info(nullptr),
      name() {}

IdentifierInfo::IdentifierInfo(const char *start, const char *end)
    : tokKind(TokenKind::TK_IDENTIFIER),
      info(nullptr),
      name(start, end - start) {}

TokenKind IdentifierInfo::getTokKind() const {
    return tokKind;
}

std::string_view IdentifierInfo::getName() const {
    return name;
}

void IdentifierInfo::setInfo(void *T) {
    info = T;
}

IdentifierInfo *IdentifierTable::get(const char *start, const char *end) {
    IdentifierInfo *&info = hashTable[std::string_view(start, end - start)];

    if (info) return info;

    // Lookups failed, make a new IdentifierInfo.
    info = new IdentifierInfo(start, end);
    return info;
}