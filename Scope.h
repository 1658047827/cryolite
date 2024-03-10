#ifndef _CRYOLITE_SCOPE_H_
#define _CRYOLITE_SCOPE_H_

#include <cassert>
#include <string>
#include <unordered_map>
#include <unordered_set>

class IdentifierInfo {
public:
    std::string getName() const { return name; }

    template <typename T>
    T *getInfo() const { return static_cast<T *>(info); }
    void setInfo(void *T) { info = T; }

private:
    std::string name;
    void *info; // Managed by the frontend.
};

class IdentifierTable {
public:
    IdentifierInfo get() {}

private:
    std::unordered_map<std::string, IdentifierInfo *> hashTable;
};

class Scope {
public:
    enum ScopeFlags : unsigned char {
        // FUNC_SCOPE - Function scope, labels are set here.
        FUNC_SCOPE = 1 << 0,

        // BREAK_SCOPE - This is a while, do, switch or for
        // that can have break stmts embedded into it.
        BREAK_SCOPE = 1 << 1,

        // CONTINUE_SCOPE - This is a while, do or for
        // that can have continue stmts embedded into it.
        CONTINUE_SCOPE = 1 << 2,

        // DECL_SCOPE - This is a scope that can contain a declaration.
        DECL_SCOPE = 1 << 3,

        // CONTROL_SCOPE - The controlling scope in a if/switch/while/for statement.
        CONTROL_SCOPE = 1 << 4,

        // RECORD_SCOPE - The scope of a struct/union definition.
        RECORD_SCOPE = 1 << 5,

        // BLOCK_SCOPE - Block scope.
        BLOCK_SCOPE = 1 << 6,

        // FUNC_PROTOTYPE_SCOPE - This is a scope that corresponds to the
        // parameters within a function prototype.
        FUNC_PROTOTYPE_SCOPE = 1 << 7
    };

    Scope(Scope *parent, unsigned char scopeFlags) {
        init(parent, scopeFlags);
    }

    void init(Scope *parent, unsigned char scopeFlags) {
        anyParent = parent;
        depth = anyParent ? anyParent->depth + 1 : 0;
        flags = scopeFlags;

        if (anyParent) {
            funcParent = anyParent->funcParent;
            breakParent = anyParent->funcParent;
            continueParent = anyParent->continueParent;
            controlParent = anyParent->controlParent;
            blockParent = anyParent->blockParent;
            withinElse = anyParent->withinElse;
        } else {
            funcParent = nullptr;
            breakParent = nullptr;
            continueParent = nullptr;
            controlParent = nullptr;
            blockParent = nullptr;
            withinElse = false;
        }

        // If this scope is a function or contains breaks/continues, remember it.
        if (flags & FUNC_SCOPE) funcParent = this;
        if (flags & BREAK_SCOPE) breakParent = this;
        if (flags & CONTINUE_SCOPE) continueParent = this;
        if (flags & CONTROL_SCOPE) controlParent = this;
        if (flags & BLOCK_SCOPE) blockParent = this;
        declsInScope.clear();
        entity = nullptr;
    }

    unsigned char getFlags() const { return flags; }
    bool isBlockScope() const { return flags & BLOCK_SCOPE; }
    bool isRecordScope() const { return flags & RECORD_SCOPE; }
    bool isFuncPrototypeScope() const { return flags & FUNC_PROTOTYPE_SCOPE; }
    bool isWithinElse() const { return withinElse; }
    void setWithinElse(bool w) { withinElse = w; }
    Scope *getParent() { return anyParent; }
    Scope *getFuncParent() { return funcParent; }
    Scope *getControlParent() { return controlParent; }
    Scope *getBlockParent() { return blockParent; }
    void *getEntity() { return entity; }
    void setEntity(void *e) { entity = e; }

    // getContinueParent - Return the closest scope that a continue statement
    // would be affected by. If the closest scope is a block scope, we know
    // that there is no loop *inside* the block.
    Scope *getContinueParent() {
        if (continueParent && !continueParent->isBlockScope())
            return continueParent;
        return nullptr;
    }

    // getBreakParent - Return the closest scope that a break statement
    // would be affected by.  If the closest scope is a block scope, we know
    // that there is no loop *inside* the block.
    Scope *getBreakParent() {
        if (breakParent && !breakParent->isBlockScope())
            return breakParent;
        return nullptr;
    }

    // isDeclScope - Return true if this is the scope that the specified decl is
    // declared in.
    bool isDeclScope(Decl *d) {
        return declsInScope.count(d) != 0;
    }

    std::unordered_set<Decl *> &getDeclsInScope() { return declsInScope; }
    bool declEmpty() const { return declsInScope.empty(); }
    void addDecl(Decl *d) { declsInScope.insert(d); }
    void removeDecl(Decl *d) { declsInScope.erase(d); }

private:
    // parent - The parent scope for this scope.
    // This is null for the translation-unit scope.
    Scope *anyParent;

    // depth - This is the depth of this scope.
    // The translation-unit scope has depth 0.
    unsigned depth;

    // flags - Contains a set of ScopeFlags.
    unsigned char flags;

    // withinElse -  Whether this scope is part of the "else" branch in
    // its parent ControlScope.
    bool withinElse;

    // TODO: Add more comments.
    Scope *funcParent;
    Scope *breakParent, *continueParent;
    Scope *controlParent;
    Scope *blockParent;

    std::unordered_set<Decl *> declsInScope;

    void *entity;
};

#endif