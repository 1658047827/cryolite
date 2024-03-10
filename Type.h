#ifndef _CRYOLITE_TYPE_H_
#define _CRYOLITE_TYPE_H_

#include <string>
#include <vector>

class Type;
class Expr;
class TypedefDecl;
class RecordDecl;
class EnumDecl;
class ArrayType;

/**
 * QualType - A (possibly-)qualified type, which is a simple wrapper class.
 */
class QualType {
public:
    enum Qualifier : unsigned char {
        CONST = 1 << 0,
        RESTRICT = 1 << 1,
        VOLATILE = 1 << 2,
    };

    QualType() : type(nullptr), quals(0) {}
    QualType(Type *type, unsigned char quals = 0) : type(type), quals(quals) {}

    Type *getTypePtr() const { return type; }
    QualType getUnqualifiedType() const { return QualType(type); }

    Type &operator*() const { return *type; }
    Type *operator->() const { return type; }

    bool isConstQualified() const { return quals & CONST; }
    bool isRestrictQualified() const { return quals & RESTRICT; }
    bool isVolatileQualified() const { return quals & VOLATILE; }

    bool isNull() const { return type == nullptr; }
    bool hasQualifiers() const { return quals != 0; }

    std::pair<std::string, std::string> repr();

    bool operator==(const QualType &other) {
        return (quals == other.quals) && (type == other.type);
    }
    bool operator!=(const QualType &other) {
        return (quals != other.quals) || (type != other.type);
    }

private:
    unsigned char quals;
    Type *type;
};

enum TypeKind {
    VOID,
    ARITH,
    POINTER,
    ARRAY,
    FUNCTION,
    RECORD,
    ENUM,
    TYPEDEF
};

class Type {
public:
    Type(TypeKind kind, QualType canonical) : kind(kind) {
        canonicalType = canonical.isNull() ? QualType(this) : canonical;
    }

    // repr - Return two string which constitutes the type declaration for an IDENTIFIER.
    virtual std::pair<std::string, std::string> repr() = 0;

    TypeKind getKind() const { return kind; }

    template <typename T>
    const T *getAs() {
        static_assert(!std::integral_constant < bool, std::is_same<T, ArrayType>::value || std::is_base_of<ArrayType, T>::value > ::value,
                      "ArrayType cannot be used with getAs");
        // If this is directly a T type, return it.
        if (const auto *ty = dynamic_cast<T *>(this))
            return ty;
        // TODO: Ref Clang Type.h
        return nullptr;
    }

    // isCanonicalUnqualified - Determines if this type would be canonical
    // if it had no further qualification.
    bool isCanonicalUnqualified() const;

    virtual bool isVoidType() const { return false; }
    virtual bool isArithmeticType() const { return false; }
    virtual bool isSignedIntegerType() const { return false; }
    virtual bool isUnsignedIntegerType() const { return false; }
    virtual bool isFloatingType() const { return false; }
    virtual bool isFunctionType() const { return false; }
    virtual bool isArrayType() const { return false; }

private:
    // canonicalType - A canonical type is the type with any typedef names
    // stripped out of it or the types it references.
    // A central concept with types is that each type always has a canonical type.
    QualType canonicalType;

    TypeKind kind;
};

/**
 * VoidType - [C99 6.2.5p19] The void type comprises an empty set of values.
 * It is an incomplete object type that cannot be completed.
 */
class VoidType : public Type {
public:
    VoidType() : Type(TypeKind::VOID, QualType()) {}

    std::pair<std::string, std::string> repr() { return std::make_pair("void", ""); }

    bool isVoidType() const { return true; }
};

class ArithType : public Type {
public:
    enum ArithKind {
#define ARITH(T, BITSIZE, REPR) T,
#include "ArithType.def"
    };

    ArithType(ArithKind kind) : Type(TypeKind::ARITH, QualType()), arithKind(kind) {}

    std::pair<std::string, std::string> repr();

    ArithKind getArithKind() const { return arithKind; }

    bool isArithmeticType() const { return true; }
    bool isSignedIntegerType() const;
    bool isUnsignedIntegerType() const;
    bool isFloatingType() const;

private:
    ArithKind arithKind;
};

class PointerType : public Type {
public:
    // The completeness of a pointer is independent of the definition status of the data type it points to.
    PointerType(QualType p, QualType canonicalPtr) : Type(TypeKind::POINTER, canonicalPtr), pointee(p) {}

    std::pair<std::string, std::string> repr();

    QualType pointee;
};

class ArrayType : public Type {
public:
    enum ArrayKind {
        CONSTANT,
        VARIABLE,
    };

    ArrayType(QualType type, QualType canonicalTy, Expr *expr = nullptr, ArrayKind kind = CONSTANT)
        : Type(TypeKind::ARRAY, canonicalTy), elemType(type), arrKind(kind), sizeExpr(expr) {}

    bool isArrayType() const { return true; }

    ArrayKind arrKind;
    QualType elemType;
    // sizeExpr - The corresponding expression of array size.
    // If we declare like "int arr[] = {0, 1};", this will be set to nullptr.
    // And "hello world" has a 'char[12]' type, whose sizeExpr is nullptr.
    Expr *sizeExpr;
};

class ConstantArrayType : public ArrayType {
public:
    ConstantArrayType(QualType type, QualType canonicalTy, std::size_t size, Expr *expr = nullptr)
        : ArrayType(type, canonicalTy, expr), size(size) {}

    std::pair<std::string, std::string> repr();

private:
    // size - The length of the array, the quantity of elements.
    std::size_t size;
};

class VariableArrayType : public ArrayType {
public:
};

/**
 * FunctionType - Represents a function prototype with parameter type info.
 * Old-style function, which has no information available about its arguments,
 * is totally not supported.
 */
class FunctionType : public Type {
public:
    FunctionType(QualType type, QualType canonicalTy, bool variadic = false)
        : Type(TypeKind::FUNCTION, canonicalTy), retType(type), isVariadic(variadic) {}

    bool isFunctionType() const { return true; }

private:
    QualType retType;
    std::vector<QualType> params;
    bool isVariadic;
};

// Struct or union
class RecordType : public Type {
public:
    // TODO: Completeness, typeSize.
    RecordType(RecordDecl *d) : Type(TypeKind::RECORD, QualType()), decl(d) {}

    RecordDecl *getDecl() const { return decl; }

    std::pair<std::string, std::string> repr();

private:
    RecordDecl *decl;
};

/**
 * EnumType - Enumeration type.
 * The underlying integer type is int by default.
 */
class EnumType : public Type {
public:
    EnumType(EnumDecl *d) : Type(TypeKind::ENUM, QualType()), decl(d) {}

    EnumDecl *getDecl() const { return decl; }

    bool isArithmeticType() const { return true; }
    // Underlying type of enum is int in implementation.
    bool isSignedIntegerType() const { return true; }

private:
    EnumDecl *decl;
};

class TypedefType : public Type {
public:
    TypedefDecl *getDecl() const { return decl; }

private:
    TypedefDecl *decl;
};

#endif