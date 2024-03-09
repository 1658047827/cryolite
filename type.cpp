#include "type.h"
#include "ast.h"
#include <algorithm>
#include <cassert>

std::pair<std::string, std::string> QualType::repr() {
    // TODO: A proper way to print qualifiers.
    auto reprPair = type->repr();
    return reprPair;
}

bool Type::isCanonicalUnqualified() const {
    // return canonicalType == QualType(this, 0);
    return canonicalType.getTypePtr() == this &&
           !canonicalType.hasQualifiers();
}

std::pair<std::string, std::string> ArithType::repr() {
    std::string s;
    switch (arithKind) {
#define ARITH(T, BITSIZE, REPR) \
    case T:                     \
        s = REPR;               \
        break;
#include "arithType.def"
    default:
        assert(0 && "Unknown builtin type");
    }
    return std::make_pair(s, "");
}

bool ArithType::isSignedIntegerType() const {
    return arithKind >= ArithKind::CHAR_S &&
           arithKind <= ArithKind::LONG_LONG;
}

bool ArithType::isUnsignedIntegerType() const {
    return arithKind >= ArithKind::BOOL &&
           arithKind <= ArithKind::UNSIGNED_LONG_LONG;
}

bool ArithType::isFloatingType() const {
    return arithKind >= ArithKind::FLOAT &&
           arithKind <= ArithKind::LONG_DOUBLE;
}

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

std::pair<std::string, std::string> PointerType::repr() {
    auto reprPair = pointee.repr();
    // Handle things like 'int (*A)[4];' correctly.
    if (pointee->getKind() == TypeKind::ARRAY) {
        reprPair.first.push_back('(');
        reprPair.second.insert(0, ")");
    }
    reprPair.first.push_back('*');
    return reprPair;
}

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

std::pair<std::string, std::string> RecordType::repr() {
    assert(0 && "Not implemented yet");
}
