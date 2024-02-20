#ifndef _CRYOLITE_TYPE_H_
#define _CRYOLITE_TYPE_H_

#include <string>
#include <vector>

class Expr;
class RecordDecl;
class EnumDecl;

enum TypeKind {
    VOID,
    BUILTIN,
    POINTER,
    ARRAY,
    FUNCTION,
    RECORD,
};

class Type {
public:
    Type(TypeKind kind, bool complete = true, size_t typeSize = 0ULL)
        : kind(kind), complete(complete), typeSize(typeSize) {}

    // repr - Return two string, "left-string + IDENTIFIER + right-string"
    // constitutes the type declaration for this IDENTIFIER. e.g.:
    // char ( * IDENTIFIER )
    // int * IDENTIFIER ( )
    // double ( * IDENTIFIER ) ( float )
    // int IDENTIFIER [ 10 ]
    virtual std::pair<std::string, std::string> repr() = 0;
    virtual size_t getTypeSize() = 0;

    bool isArithmeticType() const;

    TypeKind kind;
    // complete - A type is considered incomplete if its size or structure is unknown at a particular point in time.
    bool complete;
    // typeSize - To get the size of a specific type, we should call getTypeSize().
    // getTypeSize() will calculate the size and save the result in typeSize.
    // If typeSize!=0, getTypeSize() will return typeSize directly.
    size_t typeSize;
};

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

    Type &operator*() const { return *type; }
    Type *operator->() const { return type; }

    bool isConstQualified() const { return quals & CONST; }
    bool isRestrictQualified() const { return quals & RESTRICT; }
    bool isVolatileQualified() const { return quals & VOLATILE; }

    std::pair<std::string, std::string> repr();

    unsigned char quals;
    Type *type;
};

/**
 * VoidType - [C99 6.2.5p19] The void type comprises an empty set of values;
 * it is an incomplete object type that cannot be completed.
 */
class VoidType : public Type {
public:
    static VoidType *getVoidType();
    std::pair<std::string, std::string> repr() { return std::make_pair("void", ""); }
    size_t getTypeSize() { return 1ULL; }

protected:
    // void is incomplete.
    VoidType() : Type(TypeKind::VOID, false, 1ULL) {}
};

/**
 * BuiltinType - Arithmetic type.
 *
 * 1. TODO: Support bool type.
 *
 * 2. Temporarily, we specify 'char' as 8 bits, 'short' as 16 bits, 'int' as 32 bits,
 * 'long' as 64 bits, 'long long' as 64 bits, 'float' as 32 bits, 'double' as 64 bits
 * and 'long double' as 64 bits. Compiler typically provides options to allow users
 * to control the width of integer types, which may be implemented in the future.
 *
 * 3. Temporarily, char is considered as signed char. Compile option to control this
 * will be implemented in the future.
 */
class BuiltinType : public Type {
public:
    enum BuiltinKind {
#define BUILTIN(T, SIZE, REPR) T,
#include "builtinType.def"
    };

    static BuiltinType *getBuiltinType(BuiltinKind kind);

    std::pair<std::string, std::string> repr();
    size_t getTypeSize() { return typeSize; }

    // convRank - [C99 6.3.1.1p1]
    int convRank();

    // integerPromote - [C99 6.3.1.1p2]
    // Make sure that t is an integer type.
    static BuiltinType *integerPromote(BuiltinType *t);

    // usualArithConv - [C99 6.3.1.8] Usual arithmetic conversions.
    // All kinds of builtin arithmetic type is gotten by getBuiltinType.
    // Pointers of same builtin type point to the same static object.
    // So we can use raw pointer casually.
    static BuiltinType *usualArithConv(BuiltinType *l, BuiltinType *r);

    BuiltinKind builtinKind;

protected:
    BuiltinType(BuiltinKind kind, size_t size);
};

class PointerType : public Type {
public:
    PointerType(const QualType &p);

    std::pair<std::string, std::string> repr();
    size_t getTypeSize() { return typeSize; }

    QualType pointee;
};

class ArrayType : public Type {
public:
    enum ArrayKind {
        CONSTANT,
        VARIABLE,
    };

    ArrayType(const QualType &type, ArrayKind kind = CONSTANT, Expr *expr = nullptr);

    ArrayKind arrKind;
    QualType elemType;
    // sizeExpr - The corresponding expression of array size.
    // If we declare like "int arr[] = {0, 1};", this will be set to nullptr.
    // And "hello world" has a 'char[12]' type, whose sizeExpr is nullptr.
    Expr *sizeExpr;
};

class ConstantArrayType : public ArrayType {
public:
    ConstantArrayType(const QualType &type, size_t size, Expr *expr = nullptr);

    std::pair<std::string, std::string> repr();
    size_t getTypeSize();

    // size - The length of the array, the quantity of elements.
    size_t size;
};

class VariableArrayType : public ArrayType {
public:
};

/**
 * FunctionType - Represents a function prototype with parameter type info.
 * e.g.:
 * "int foo(int)", "float bar()" or "long func(char *, int)"
 * Old-style function, which has no information available about its arguments,
 * is totally not supported. So all prototypes are complete.
 */
class FunctionType : public Type {
public:
    FunctionType(const QualType &type, bool variadic = false);

    size_t getTypeSize() { return 1ULL; }

    QualType retType;
    std::vector<QualType> params;
    bool isVariadic;
};

// Struct or union
class RecordType : public Type {
public:
    // TODO: Completeness, typeSize.
    RecordType(RecordDecl *d) : Type(TypeKind::RECORD), decl(d) {}

    std::pair<std::string, std::string> repr();
    size_t getTypeSize() { return 0ULL; }

    RecordDecl *decl;
};

class EnumType : public Type {
public:
    EnumDecl *decl;
};

#endif