/**
 * idtable.h - Definition of IdentifierTable.
 *
 * This header file defines the identifier table, which is used in building AST and semantic analysis.
 */

#ifndef _CRYOLITE_IDTABLE_H_
#define _CRYOLITE_IDTABLE_H_

#include <cassert>
#include <string>
#include <unordered_map>

class IdentifierInfo {
public:
    std::string name;
};

class IdentifierTable {
public:
    IdentifierTable() = default;

    // get - Get a entry by string key.
    // Return nullptr when the key does not exist.
    IdentifierInfo *get(const std::string &key) {
        auto iter = table.find(key);
        if (iter == table.end()) {
            return nullptr;
        } else {
            return iter->second;
        }
    }

    // insert - Try to insert a new entry into table.
    // Return 0 on successful insertion, 1 on duplicate key,
    // 2 on failed insertion.
    int insert(const std::string &key, IdentifierInfo *idInfo) {
        assert(key == idInfo->name);
        if (table.count(key)) {
            return 1;
        } else {
            auto result = table.emplace(key, idInfo);
            return result.second ? 0 : 2;
        }
    }

private:
    std::unordered_map<std::string, IdentifierInfo *> table;
};

#endif