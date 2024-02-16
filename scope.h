#ifndef _CRYOLITE_SCOPE_H_
#define _CRYOLITE_SCOPE_H_

#include <cassert>
#include <string>
#include <unordered_map>

class IdentifierInfo {
public:
    std::string name;
};

enum ScopeKind {
    SK_FILE,
    SK_PROTO,
    SK_BLOCK,
    SK_FUNC
};

class Scope {
public:

private:
    std::unordered_map<std::string, IdentifierInfo*> identMap;
};

#endif