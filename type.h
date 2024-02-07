#ifndef _CRYOLITE_TYPE_H_
#define _CRYOLITE_TYPE_H_

#include <string>

class Expr;

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
    Type(TypeKind kind, bool complete = true) : kind(kind), complete(complete) {}

    // repr - Return two string, "left-string + IDENTIFIER + right-string"
    // constitutes the type declaration for this IDENTIFIER. e.g.:
    // char ( * IDENTIFIER )
    // int * IDENTIFIER ( )
    // double ( * IDENTIFIER ) ( float )
    // int IDENTIFIER [ 10 ]
    virtual std::pair<std::string, std::string> repr() = 0;

    TypeKind kind;
    // complete - A type is considered incomplete if its size or structure is unknown at a particular point in time.
    bool complete;
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

    bool isConst() { return quals & CONST; }
    bool isRestrict() { return quals & RESTRICT; }
    bool isVolatile() { return quals & VOLATILE; }

    std::pair<std::string, std::string> repr();

    unsigned char quals;
    Type *type;
};

/**
 * VoidType - [C11 6.2.5p19] The void type comprises an empty set of values;
 * it is an incomplete object type that cannot be completed.
 */
class VoidType : public Type {
public:
    static VoidType *getVoidType();
    std::pair<std::string, std::string> repr() { return std::make_pair("void", ""); }

protected:
    VoidType() : Type(TypeKind::VOID) {}
};

class BuiltinType : public Type {
public:
    enum BuiltinSpec : unsigned int {
        CHAR = 1 << 0,
        SHORT = 1 << 1,
        INT = 1 << 2,
        LONG = 1 << 3,
        LONGLONG = 1 << 4,
        FLOAT = 1 << 5,
        DOUBLE = 1 << 6,
        SIGNED = 1 << 7,
        UNSIGNED = 1 << 8
    };

    static BuiltinType *getBuiltinType(unsigned int flags);
    static bool isChar(unsigned int flags);
    static bool isSignedChar(unsigned int flags);
    static bool isUnsignedChar(unsigned int flags);
    static bool isShort(unsigned int flags);
    static bool isUnsignedShort(unsigned int flags);
    static bool isInt(unsigned int flags);
    static bool isUnsignedInt(unsigned int flags);
    static bool isLong(unsigned int flags);
    static bool isUnsignedLong(unsigned int flags);
    static bool isLongLong(unsigned int flags);
    static bool isUnsignedLongLong(unsigned int flags);
    static bool isFloat(unsigned int flags);
    static bool isDouble(unsigned int flags);
    static bool isLongDouble(unsigned int flags);

    std::pair<std::string, std::string> repr();

    // kind - Bit flags.
    unsigned int kind;

protected:
    BuiltinType(unsigned int flags);
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

    // size - The length of the array, the quantity of elements.
    size_t size;
};

class VariableArrayType : public ArrayType {
public:
};

class FunctionType : public Type {
};

// Struct or union
class RecordType : public Type {
};

#endif