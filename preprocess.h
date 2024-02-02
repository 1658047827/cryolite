#ifndef _CRYOLITE_PREPROCESS_H_
#define _CRYOLITE_PREPROCESS_H_

#include "lexer.h"
#include <map>
#include <vector>

class Macro {
public:
    // For object-like macro.
    Macro(std::list<Token> &replSeq)
        : objLike(true), variadic(false), replSeq(replSeq) {}

    // For function-like macro.
    Macro(std::list<Token> &replSeq, std::vector<std::string> &params, bool variadic)
        : objLike(false), variadic(variadic), replSeq(replSeq), params(params) {}

    ~Macro() = default;

    bool isObjLike() { return objLike; }
    bool isFuncLike() { return !objLike; }
    bool isVariadic() { return variadic; }
    std::vector<std::string> &getParams() { return params; }

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

    void handleIfdef();
    void handleIf();
    void handleError();

private:
    std::map<std::string, Macro> macroMap;
};

#endif