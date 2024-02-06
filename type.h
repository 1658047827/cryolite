#ifndef _CRYOLITE_TYPE_H_
#define _CRYOLITE_TYPE_H_

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
    Type(TypeKind kind) : kind(kind) {}
    TypeKind kind;
};

class QualType {
public:
    unsigned char quals;
    Type *type;
};

class VoidType : public Type {
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

    BuiltinType *getBuiltinType(unsigned int flags);
    bool isChar(unsigned int flags);
    bool isSignedChar(unsigned int flags);
    bool isUnsignedChar(unsigned int flags);
    bool isShort(unsigned int flags);
    bool isUnsignedShort(unsigned int flags);
    bool isInt(unsigned int flags);
    bool isUnsignedInt(unsigned int flags);
    bool isLong(unsigned int flags);
    bool isUnsignedLong(unsigned int flags);
    bool isLongLong(unsigned int flags);
    bool isUnsignedLongLong(unsigned int flags);
    bool isFloat(unsigned int flags);
    bool isDouble(unsigned int flags);
    bool isLongDouble(unsigned int flags);

    // kind - Bit flags.
    unsigned int kind;

protected:
    BuiltinType(unsigned int flags);
};

class PointerType : public Type {
};

class ArrayType : public Type {
};

class FunctionType : public Type {
};

// Struct or union
class RecordType : public Type {
};

#endif