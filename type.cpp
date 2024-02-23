#include "type.h"
#include "ast.h"
#include <algorithm>
#include <cassert>

bool Type::isArithmeticType() const {
    return kind == TypeKind::ARITH;
}

bool Type::isSignedIntegerType() {
    if (const auto *bt = dynamic_cast<ArithType *>(this))
        return bt->getArithKind() >= ArithType::CHAR_S &&
               bt->getArithKind() <= ArithType::LONG_LONG;
    // Underlying type of enum is int in implementation.
    if (const auto *et = dynamic_cast<EnumType *>(this))
        return true;
    return false;
}

bool Type::isUnsignedIntegerType() {
    if (const auto *bt = dynamic_cast<ArithType *>(this))
        return bt->getArithKind() >= ArithType::BOOL &&
               bt->getArithKind() <= ArithType::UNSIGNED_LONG_LONG;
    // Underlying type of enum is int in implementation.
    return false;
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

ArithType::ArithType(ArithKind kind, std::size_t size)
    : Type(TypeKind::ARITH, true, size), arithKind(kind) {}

ArithType *ArithType::getArithType(ArithKind kind) {
#define ARITH(T, SIZE, REPR) static ArithType *T##_TYPE = new ArithType(T, SIZE);
#include "arithType.def"

    switch (kind) {
#define ARITH(T, SIZE, REPR) \
    case T:                  \
        return T##_TYPE;
#include "arithType.def"
    default:
        assert(0 && "Unknown arith type");
    }
    // unreachable
    return nullptr;
}

std::pair<std::string, std::string> ArithType::repr() {
    std::string s;
    switch (arithKind) {
#define ARITH(T, SIZE, REPR) \
    case T:                  \
        s = REPR;            \
        break;
#include "arithType.def"
    default:
        assert(0 && "Unknown builtin type");
    }
    return std::make_pair(s, "");
}

int ArithType::convRank() {
    switch (arithKind) {
    case BOOL:
        return 1;
    case CHAR_S:
    case CHAR_U:
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

// ArithType *ArithType::integerPromote(ArithType *t) {
//     if (t->convRank() < getArithType(INT)->convRank()) {
//         return getArithType(INT);
//     } else {
//         return t;
//     }
//     // TODO: What if the width of 'int' is the same as that of 'short'?
// }

// ArithType *ArithType::usualArithConv(ArithType *l, ArithType *r) {
//     assert(l->kind == TypeKind::ARITH && r->kind == TypeKind::ARITH);

//     // If either operand has a floating-point type, then the operand with the lower conversion rank
//     // is converted to a type with the same rank as the other operand.
//     if (l->arithKind == (LONG | DOUBLE))
//         return l;
//     if (r->arithKind == (LONG | DOUBLE))
//         return r;

//     if (l->arithKind == DOUBLE)
//         return l;
//     if (r->arithKind == DOUBLE)
//         return r;

//     if (l->arithKind == FLOAT)
//         return l;
//     if (r->arithKind == FLOAT)
//         return r;

//     // If both operands are integers, integer promotion is first performed on both operands.
//     ArithType *t1 = integerPromote(l);
//     ArithType *t2 = integerPromote(r);

//     if (t1 == t2) {
//         return t1;
//     } else if ((t1->arithKind & UNSIGNED) == (t2->arithKind & UNSIGNED)) {
//         // if T1 and T2 are both signed integer types or both unsigned integer types,
//         // C is the type of greater integer conversion rank.
//         return t1->convRank() > t2->convRank() ? t1 : t2;
//     } else {
//         // One type between T1 and T2 is an signed integer type S,
//         // the other type is an unsigned integer type U.
//         if (t1->arithKind & UNSIGNED)
//             std::swap(t1, t2);
//         // Now T1 is signed(S), T2 is unsigned(U).

//         if (t2->convRank() >= t1->convRank()) {
//             return t2;
//         } else if (t1->getTypeSize() > t2->getTypeSize()) {
//             return t1;
//         } else {
//             // Return the unsigned integer type corresponding to T1(S).
//             return getArithType(UNSIGNED | t1->arithKind);
//         }
//     }
// }

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

ConstantArrayType::ConstantArrayType(const QualType &type, std::size_t size, Expr *expr)
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

std::size_t ConstantArrayType::getTypeSize() {
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
