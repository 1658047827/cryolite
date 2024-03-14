#ifndef _CRYOLITE_IDENTIFIER_H_
#define _CRYOLITE_IDENTIFIER_H_

#include "TokenKind.h"
#include <string>
#include <unordered_map>

class IdentifierInfo {
public:
    IdentifierInfo();
    IdentifierInfo(const char *start, const char *end);

    TokenKind getTokKind() const;
    std::string_view getName() const;

    template <typename T>
    T *getInfo() const { return static_cast<T *>(info); }

    void setInfo(void *T);

private:
    TokenKind tokKind;
    std::string name;
    void *info; // Managed by the frontend.
};

class IdentifierTable {
public:
    IdentifierInfo *get(const char *start, const char *end);

private:
    std::unordered_map<std::string_view, IdentifierInfo *> hashTable;
};

#endif