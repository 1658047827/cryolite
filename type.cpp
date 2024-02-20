#include "type.h"
#include "ast.h"
#include <algorithm>
#include <cassert>

bool Type::isArithmeticType() const {
    return kind == TypeKind::BUILTIN;
}

std::pair<std::string, std::string> QualType::repr() {
    // TODO: A proper way to print qualifiers.
    // std::string s = isConstQualified() ? "const " : "";
    // if (isVolatileQualified())
    //     s += "volatile ";
    // if (isRestrictQualified())
    //     s += "restrict ";
    auto reprPair = type->repr();
    // reprPair.first = s + reprPair.first;
    return reprPair;
}

VoidType *VoidType::getVoidType() {
    static VoidType *voidType = new VoidType();
    return voidType;
}

BuiltinType::BuiltinType(BuiltinKind kind, size_t size)
    : Type(TypeKind::BUILTIN, true, size), builtinKind(kind) {}

BuiltinType *BuiltinType::getBuiltinType(BuiltinKind kind) {
#define BUILTIN(T, SIZE, REPR) static BuiltinType *T##_TYPE = new BuiltinType(T, SIZE);
#include "builtinType.def"

    switch (kind) {
#define BUILTIN(T, SIZE, REPR) \
    case T:                    \
        return T##_TYPE;
#include "builtinType.def"
    default:
        assert(0 && "Unknown builtin type");
    }
}

std::pair<std::string, std::string> BuiltinType::repr() {
    std::string s;
    switch (builtinKind) {
#define BUILTIN(T, SIZE, REPR) \
    case T:                    \
        s = REPR;              \
        break;
#include "builtinType.def"
    default:
        assert(0 && "Unknown builtin type");
    }
    return std::make_pair(s, "");
}

int BuiltinType::convRank() {
    switch (builtinKind) {
    // TODO: Support bool.
    case CHAR:
    case SIGNED_CHAR:
    case UNSIGNED_CHAR:
        return 2;
    case SHORT:
    case UNSIGNED_SHORT:
        return 3;
    case INT:
    case UNSIGNED_INT:
        return 4;
    case LONG:
    case UNSIGNED_LONG:
        return 5;
    case LONG_LONG:
    case UNSIGNED_LONG_LONG:
        return 6;
    case FLOAT:
        return 7;
    case DOUBLE:
        return 8;
    case LONG_DOUBLE:
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
        } else if (t1->getTypeSize() > t2->getTypeSize()) {
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
    if (pointee->kind == TypeKind::ARRAY) {
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

size_t ConstantArrayType::getTypeSize() {
    if (typeSize != 0) {
        return typeSize;
    } else {
        return typeSize = elemType->getTypeSize() * size;
    }
}

FunctionType::FunctionType(const QualType &type, bool variadic)
    : Type(TypeKind::FUNCTION, true, 1), retType(type), isVariadic(variadic) {}

std::pair<std::string, std::string> RecordType::repr() {
    std::string rcd = decl->isStruct ? "struct " : "union ";
    rcd += decl->recordName;
    return make_pair(rcd, "");
}
