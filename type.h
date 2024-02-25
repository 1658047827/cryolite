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

    Type *type;

private:
    unsigned char quals;
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
    Type(TypeKind kind, QualType canonical)
        : kind(kind), canonicalType(canonical) {}

    // repr - Return two string, "left-string + IDENTIFIER + right-string"
    // constitutes the type declaration for this IDENTIFIER. e.g.
    //     char ( * IDENTIFIER )
    //     int * IDENTIFIER ( )
    //     double ( * IDENTIFIER ) ( float )
    //     int IDENTIFIER [ 10 ]
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

    // canonicalType - A canonical type is the type with any typedef names
    // stripped out of it or the types it references.
    // A central concept with types is that each type always has a canonical type.
    QualType canonicalType;

    TypeKind kind;
    // complete - A type is considered incomplete if its size or structure is unknown at a particular point in time.
    bool complete;
    // typeSize - To get the size of a specific type, we should call getTypeSize().
    // getTypeSize() will calculate the size and save the result in typeSize.
    // If typeSize!=0, getTypeSize() will return typeSize directly.
    std::size_t typeSize;
};

/**
 * VoidType - [C99 6.2.5p19] The void type comprises an empty set of values;
 * it is an incomplete object type that cannot be completed.
 */
class VoidType : public Type {
public:
    static VoidType *getVoidType();
    std::pair<std::string, std::string> repr() { return std::make_pair("void", ""); }

    bool isVoidType() const { return true; }

private:
    VoidType() : Type(TypeKind::VOID, QualType(this)) {}
};

/**
 * ArithType - Arithmetic type.
 *
 * 1. Temporarily, we specify 'char' as 8 bits, 'short' as 16 bits, 'int' as 32 bits,
 * 'long' as 64 bits, 'long long' as 64 bits, 'float' as 32 bits, 'double' as 64 bits
 * and 'long double' as 64 bits. Compiler typically provides options to allow users
 * to control the width of integer types, which may be implemented in the future.
 *
 * 2. Temporarily, char is considered as signed char. Compile option to control this
 * will be implemented in the future.
 */
class ArithType : public Type {
public:
    enum ArithKind {
#define ARITH(T, BITSIZE, REPR) T,
#include "arithType.def"
    };

    static ArithType *getArithType(ArithKind kind);

    std::pair<std::string, std::string> repr();

    // convRank - [C99 6.3.1.1p1]
    int convRank();

    // integerPromote - [C99 6.3.1.1p2]
    // Make sure that t is an integer type.
    // static ArithType *integerPromote(ArithType *t);

    // usualArithConv - [C99 6.3.1.8] Usual arithmetic conversions.
    // All kinds of builtin arithmetic type is gotten by getArithType.
    // Pointers of same builtin type point to the same static object.
    // So we can use raw pointer casually.
    // static ArithType *usualArithConv(ArithType *l, ArithType *r);

    ArithKind getArithKind() const { return arithKind; }

    bool isArithmeticType() const { return true; }
    bool isSignedIntegerType() const;
    bool isUnsignedIntegerType() const;
    bool isFloatingType() const;

private:
    ArithKind arithKind;

    ArithType(ArithKind kind) : Type(TypeKind::ARITH, QualType(this)), arithKind(kind) {}
};

class PointerType : public Type {
public:
    PointerType(const QualType &p);

    std::pair<std::string, std::string> repr();

    QualType pointee;
};

class ArrayType : public Type {
public:
    enum ArrayKind {
        CONSTANT,
        VARIABLE,
    };

    ArrayType(const QualType &type, ArrayKind kind = CONSTANT, Expr *expr = nullptr);

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
    ConstantArrayType(const QualType &type, std::size_t size, Expr *expr = nullptr);

    std::pair<std::string, std::string> repr();

    // size - The length of the array, the quantity of elements.
    std::size_t size;
};

class VariableArrayType : public ArrayType {
public:
};

/**
 * FunctionType - Represents a function prototype with parameter type info.
 * e.g.
 * "int foo(int)", "float bar()" or "long func(char *, int)"
 * Old-style function, which has no information available about its arguments,
 * is totally not supported. So all prototypes are complete.
 */
class FunctionType : public Type {
public:
    FunctionType(const QualType &type, bool variadic = false);

    bool isFunctionType() const { return true; }

    QualType retType;
    std::vector<QualType> params;
    bool isVariadic;
};

// Struct or union
class RecordType : public Type {
public:
    // TODO: Completeness, typeSize, canonicalType.
    RecordType(RecordDecl *d) : Type(TypeKind::RECORD, QualType()), decl(d) {}

    RecordDecl *getDecl() const { return decl; }

    std::pair<std::string, std::string> repr();
    std::size_t getTypeSize() { return 0ULL; }

private:
    RecordDecl *decl;
};

/**
 * EnumType - Enumeration type.
 * The underlying integer type is int by default.
 */
class EnumType : public Type {
public:
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