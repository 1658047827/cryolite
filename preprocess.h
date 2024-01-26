#ifndef _CRYOLITH_PREPROCESS_H_
#define _CRYOLITH_PREPROCESS_H_

#include <map>
#include <string>
#include <vector>

#include "lexer.h"

class Macro {
public:
    // For object-like macro.
    Macro(std::list<Token> &replSeq)
        : objLike(true), variadic(false), replSeq(replSeq) {}

    // For function-like macro.
    Macro(std::list<Token> &replSeq, std::vector<std::string> &params, bool variadic)
        : objLike(false), variadic(variadic), replSeq(replSeq), params(params) {}

    ~Macro() = default;

    bool IsObjLike() { return objLike; }
    bool IsFuncLike() { return !objLike; }
    bool IsVariadic() { return variadic; }
    std::vector<std::string> &Params() { return params; }

private:
    bool objLike;
    bool variadic;
    // replSeq - Replacement token sequence.
    std::list<Token> replSeq;
    std::vector<std::string> params;
};

struct PPCondition {
    // ifLoc - Location where the conditional started.
    SourceLocation ifLoc;
};

class Preprocessor {
public:
    ~Preprocessor() = default;

    void HandleIfdef();
    void HandleIf();
    void HandleError();

private:
    std::map<std::string, Macro> macroMap;
};

#endif