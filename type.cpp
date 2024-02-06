#include "type.h"
#include <algorithm>

BuiltinType::BuiltinType(unsigned int flags)
    : Type(TypeKind::BUILTIN), kind(flags) {}

const unsigned int shortVec[4] = {
    BuiltinType::SHORT,
    BuiltinType::SHORT | BuiltinType::INT,
    BuiltinType::SIGNED | BuiltinType::SHORT,
    BuiltinType::SIGNED | BuiltinType::SHORT | BuiltinType::INT,
};

const unsigned int unsignedShortVec[2] = {
    BuiltinType::UNSIGNED | BuiltinType::SHORT,
    BuiltinType::UNSIGNED | BuiltinType::SHORT | BuiltinType::INT,
};

const unsigned int intVec[3] = {
    BuiltinType::INT,
    BuiltinType::SIGNED,
    BuiltinType::SIGNED | BuiltinType::INT,
};

const unsigned int unsignedIntVec[2] = {
    BuiltinType::UNSIGNED,
    BuiltinType::UNSIGNED | BuiltinType::INT,
};

const unsigned int longVec[4] = {
    BuiltinType::LONG,
    BuiltinType::LONG | BuiltinType::INT,
    BuiltinType::SIGNED | BuiltinType::LONG,
    BuiltinType::SIGNED | BuiltinType::LONG | BuiltinType::INT,
};

const unsigned int unsignedLongVec[2] = {
    BuiltinType::UNSIGNED | BuiltinType::LONG,
    BuiltinType::UNSIGNED | BuiltinType::LONG | BuiltinType::INT,
};

const unsigned int longLongVec[4] = {
    BuiltinType::LONGLONG,
    BuiltinType::LONGLONG | BuiltinType::INT,
    BuiltinType::SIGNED | BuiltinType::LONGLONG,
    BuiltinType::SIGNED | BuiltinType::LONGLONG | BuiltinType::INT,
};

const unsigned int unsignedLongLongVec[2] = {
    BuiltinType::UNSIGNED | BuiltinType::LONGLONG,
    BuiltinType::UNSIGNED | BuiltinType::LONGLONG | BuiltinType::INT,
};

BuiltinType *BuiltinType::getBuiltinType(unsigned int flags) {
#define BUILTIN_TYPE(tag) new BuiltinType(tag)
    static BuiltinType *charType = BUILTIN_TYPE(CHAR);
    static BuiltinType *signedCharType = BUILTIN_TYPE(SIGNED | CHAR);
    static BuiltinType *unsignedCharType = BUILTIN_TYPE(UNSIGNED | CHAR);
    static BuiltinType *shortType = BUILTIN_TYPE(SHORT);
    static BuiltinType *unsignedShortType = BUILTIN_TYPE(UNSIGNED | SHORT);
    static BuiltinType *intType = BUILTIN_TYPE(INT);
    static BuiltinType *unsignedIntType = BUILTIN_TYPE(UNSIGNED | INT);
    static BuiltinType *longType = BUILTIN_TYPE(LONG);
    static BuiltinType *unsignedLongType = BUILTIN_TYPE(UNSIGNED | LONG);
    static BuiltinType *longLongType = BUILTIN_TYPE(LONGLONG);
    static BuiltinType *unsignedLongLongType = BUILTIN_TYPE(UNSIGNED | LONGLONG);
    static BuiltinType *floatType = BUILTIN_TYPE(FLOAT);
    static BuiltinType *doubleType = BUILTIN_TYPE(DOUBLE);
    static BuiltinType *longDoubleType = BUILTIN_TYPE(LONG | DOUBLE);
#undef BUILTIN_TYPE

    if (isChar(flags)) return charType;
    if (isSignedChar(flags)) return signedCharType;
    if (isUnsignedChar(flags)) return unsignedCharType;
    if (isShort(flags)) return shortType;
    if (isUnsignedShort(flags)) return unsignedShortType;
    if (isInt(flags)) return intType;
    if (isUnsignedInt(flags)) return unsignedIntType;
    if (isLong(flags)) return longType;
    if (isUnsignedLong(flags)) return unsignedLongType;
    if (isLongLong(flags)) return longLongType;
    if (isUnsignedLongLong(flags)) return unsignedLongLongType;
    if (isFloat(flags)) return floatType;
    if (isDouble(flags)) return doubleType;
    if (isLongDouble(flags)) return longDoubleType;
    return nullptr;
}

bool BuiltinType::isChar(unsigned int flags) {
    return flags == CHAR;
}

bool BuiltinType::isSignedChar(unsigned int flags) {
    return flags == (SIGNED | CHAR);
}

bool BuiltinType::isUnsignedChar(unsigned int flags) {
    return flags == (UNSIGNED | CHAR);
}

bool BuiltinType::isShort(unsigned int flags) {
    return std::find(std::begin(shortVec), std::end(shortVec), flags) != std::end(shortVec);
}

bool BuiltinType::isUnsignedShort(unsigned int flags) {
    return std::find(std::begin(unsignedShortVec), std::end(unsignedShortVec), flags) != std::end(unsignedShortVec);
}

bool BuiltinType::isInt(unsigned int flags) {
    return std::find(std::begin(intVec), std::end(intVec), flags) != std::end(intVec);
}

bool BuiltinType::isUnsignedInt(unsigned int flags) {
    return std::find(std::begin(unsignedIntVec), std::end(unsignedIntVec), flags) != std::end(unsignedIntVec);
}

bool BuiltinType::isLong(unsigned int flags) {
    return std::find(std::begin(longVec), std::end(longVec), flags) != std::end(longVec);
}

bool BuiltinType::isUnsignedLong(unsigned int flags) {
    return std::find(std::begin(unsignedLongVec), std::end(unsignedLongVec), flags) != std::end(unsignedLongVec);
}

bool BuiltinType::isLongLong(unsigned int flags) {
    return std::find(std::begin(longLongVec), std::end(longLongVec), flags) != std::end(longLongVec);
}

bool BuiltinType::isUnsignedLongLong(unsigned int flags) {
    return std::find(std::begin(unsignedLongLongVec), std::end(unsignedLongLongVec), flags) != std::end(unsignedLongLongVec);
}

bool BuiltinType::isFloat(unsigned int flags) {
    return flags == FLOAT;
}

bool BuiltinType::isDouble(unsigned int flags) {
    return flags == DOUBLE;
}

bool BuiltinType::isLongDouble(unsigned int flags) {
    return flags == (LONG | DOUBLE);
}
