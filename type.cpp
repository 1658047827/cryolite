#include "type.h"
#include "ast.h"
#include <algorithm>
#include <cassert>

std::pair<std::string, std::string> QualType::repr() {
    // TODO: A proper way to print qualifiers.
    // std::string s = isConst() ? "const " : "";
    // if (isVolatile())
    //     s += "volatile ";
    // if (isRestrict())
    //     s += "restrict ";
    auto reprPair = type->repr();
    // reprPair.first = s + reprPair.first;
    return reprPair;
}

VoidType *VoidType::getVoidType() {
    static VoidType *voidType = new VoidType();
    return voidType;
}

BuiltinType::BuiltinType(unsigned int flags, size_t size)
    : Type(TypeKind::BUILTIN, true, size), builtinKind(flags) {}

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
    static BuiltinType *charType = new BuiltinType(CHAR, 1);
    static BuiltinType *signedCharType = new BuiltinType((SIGNED | CHAR), 1);
    static BuiltinType *unsignedCharType = new BuiltinType((UNSIGNED | CHAR), 1);
    static BuiltinType *shortType = new BuiltinType(SHORT, 2);
    static BuiltinType *unsignedShortType = new BuiltinType((UNSIGNED | SHORT), 2);
    static BuiltinType *intType = new BuiltinType(INT, 4);
    static BuiltinType *unsignedIntType = new BuiltinType((UNSIGNED | INT), 4);
    static BuiltinType *longType = new BuiltinType(LONG, 4);
    static BuiltinType *unsignedLongType = new BuiltinType((UNSIGNED | LONG), 4);
    static BuiltinType *longLongType = new BuiltinType(LONGLONG, 8);
    static BuiltinType *unsignedLongLongType = new BuiltinType((UNSIGNED | LONGLONG), 8);
    static BuiltinType *floatType = new BuiltinType(FLOAT, 4);
    static BuiltinType *doubleType = new BuiltinType(DOUBLE, 8);
    static BuiltinType *longDoubleType = new BuiltinType((LONG | DOUBLE), 8);

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

bool BuiltinType::isInteger() {
    if (builtinKind & (CHAR | SHORT | INT | LONGLONG) || builtinKind == LONG) {
        return true;
    } else {
        return false;
    }
}

bool BuiltinType::isFloating() {
    return builtinKind & (FLOAT | DOUBLE);
}

std::pair<std::string, std::string> BuiltinType::repr() {
    std::string s;
    switch (builtinKind) {
    case CHAR:
        s = "char";
        break;
    case (SIGNED | CHAR):
        s = "signed char";
        break;
    case (UNSIGNED | CHAR):
        s = "unsigned char";
        break;
    case SHORT:
        s = "short";
        break;
    case (UNSIGNED | SHORT):
        s = "unsigned short";
        break;
    case INT:
        s = "int";
        break;
    case (UNSIGNED | INT):
        s = "unsigned int";
        break;
    case LONG:
        s = "long";
        break;
    case (UNSIGNED | LONG):
        s = "unsigned long";
        break;
    case LONGLONG:
        s = "long long";
        break;
    case (UNSIGNED | LONGLONG):
        s = "unsigned long long";
        break;
    case FLOAT:
        s = "float";
        break;
    case DOUBLE:
        s = "double";
        break;
    case (LONG | DOUBLE):
        s = "long double";
        break;
    }
    return std::make_pair(s, "");
}

int BuiltinType::convRank() {
    switch (builtinKind) {
    // TODO: Support bool.
    case CHAR:
    case (SIGNED | CHAR):
    case (UNSIGNED | CHAR):
        return 2;
    case SHORT:
    case (UNSIGNED | SHORT):
        return 3;
    case INT:
    case (UNSIGNED | INT):
        return 4;
    case LONG:
    case (UNSIGNED | LONG):
        return 5;
    case LONGLONG:
    case (UNSIGNED | LONGLONG):
        return 6;
    case FLOAT:
        return 7;
    case DOUBLE:
        return 8;
    case (LONG | DOUBLE):
        return 9;
    default:
        assert(0);
    }
}

BuiltinType *BuiltinType::integerPromote(BuiltinType *t) {
    if (t->convRank() < getBuiltinType(INT)->convRank()) {
        return getBuiltinType(INT);
    } else {
        return t;
    }
    // TODO: What if the width of 'int' is the same as that of 'short'?
}

BuiltinType *BuiltinType::usualArithConv(BuiltinType *l, BuiltinType *r) {
    assert(l->kind == TypeKind::BUILTIN && r->kind == TypeKind::BUILTIN);

    // If either operand has a floating-point type, then the operand with the lower conversion rank
    // is converted to a type with the same rank as the other operand.
    if (l->builtinKind == (LONG | DOUBLE))
        return l;
    if (r->builtinKind == (LONG | DOUBLE))
        return r;

    if (l->builtinKind == DOUBLE)
        return l;
    if (r->builtinKind == DOUBLE)
        return r;

    if (l->builtinKind == FLOAT)
        return l;
    if (r->builtinKind == FLOAT)
        return r;

    // If both operands are integers, integer promotion is first performed on both operands.
    BuiltinType *t1 = integerPromote(l);
    BuiltinType *t2 = integerPromote(r);

    if (t1 == t2) {
        return t1;
    } else if ((t1->builtinKind & UNSIGNED) == (t2->builtinKind & UNSIGNED)) {
        // if T1 and T2 are both signed integer types or both unsigned integer types,
        // C is the type of greater integer conversion rank.
        return t1->convRank() > t2->convRank() ? t1 : t2;
    } else {
        // One type between T1 and T2 is an signed integer type S,
        // the other type is an unsigned integer type U.
        if (t1->builtinKind & UNSIGNED)
            std::swap(t1, t2);
        // Now T1 is signed(S), T2 is unsigned(U).

        if (t2->convRank() >= t1->convRank()) {
            return t2;
        } else if (t1->getSize() > t2->getSize()) {
            return t1;
        } else {
            // Return the unsigned integer type corresponding to T1(S).
            return getBuiltinType(UNSIGNED | t1->builtinKind);
        }
    }
}

// The completeness of a pointer is independent of the definition status of the data type it points to.
// TODO: The size of pointer.
PointerType::PointerType(const QualType &p)
    : Type(TypeKind::POINTER, true, 8), pointee(p) {}

std::pair<std::string, std::string> PointerType::repr() {
    auto reprPair = pointee.repr();
    // Handle things like 'int (*A)[4];' correctly.
    if (pointee.type->kind == TypeKind::ARRAY) {
        reprPair.first.push_back('(');
        reprPair.second.insert(0, ")");
    }
    reprPair.first.push_back('*');
    return reprPair;
}

ArrayType::ArrayType(const QualType &type, ArrayKind kind, Expr *expr)
    : Type(TypeKind::ARRAY), elemType(type), arrKind(kind), sizeExpr(expr) {}

ConstantArrayType::ConstantArrayType(const QualType &type, size_t size, Expr *expr)
    : ArrayType(type, CONSTANT, expr), size(size) {}

std::pair<std::string, std::string> ConstantArrayType::repr() {
    auto reprPair = elemType.repr();
    // TODO: [C99 6.7.5.2] Support arrays like "int A[static restrict 4]" which has
    // index type qualifier(s) and static size. These are only allowed on function parameters.
    // We can refer to TypePrinter.cpp in the Clang source code.
    std::string tmp = "[";
    tmp.append(std::to_string(size));
    tmp.push_back(']');
    reprPair.second.insert(0, tmp);
    return reprPair;
}

size_t ConstantArrayType::getSize() {
    if (sizeCache != 0) {
        return sizeCache;
    } else {
        return sizeCache = elemType.type->getSize() * size;
    }
}
