#ifndef _CRYOLITE_IDENTIFIER_H_
#define _CRYOLITE_IDENTIFIER_H_

#include <string>
#include <unordered_map>

class IdentifierInfo {
public:
    IdentifierInfo(const char *start, const char *end) : name(start, end - start) {}

    std::string_view getName() const { return name; }

    template <typename T>
    T *getInfo() const { return static_cast<T *>(info); }
    void setInfo(void *T) { info = T; }

private:
    std::string name;
    void *info; // Managed by the frontend.
};

class IdentifierTable {
public:
    IdentifierInfo &get(const char *start, const char *end) {
        IdentifierInfo *&info = hashTable[std::string_view(start, end - start)];
        if (info) return *info;

        // Lookups failed, make a new IdentifierInfo.
        info = new IdentifierInfo(start, end);
        
    }

private:
    std::unordered_map<std::string_view, IdentifierInfo *> hashTable;
};

#endif