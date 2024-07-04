#ifndef _CRYOLITE_TYPE_H_
#define _CRYOLITE_TYPE_H_

struct Type;

typedef enum Qualifier {
    CONST = 1 << 0,
    RESTRICT = 1 << 1,
    VOLATILE = 1 << 2,
} Qualifier;

typedef struct QualType {
    unsigned char quals;
    struct Type *t;
} QualType;

typedef struct VoidType {

} VoidType;

typedef enum ArithKind {
    ARITH_BOOL,
    ARITH_CHAR_U,
    ARITH_UNSIGNED_CHAR,
    ARITH_UNSIGNED_SHORT,
    ARITH_UNSIGNED_INT,
    ARITH_UNSIGNED_LONG,
    ARITH_UNSIGNED_LONG_LONG,

    ARITH_CHAR_S,
    ARITH_SIGNED_CHAR,
    ARITH_SHORT,
    ARITH_INT,
    ARITH_LONG,
    ARITH_LONG_LONG,

    ARITH_FLOAT,
    ARITH_DOUBLE,
    ARITH_LONG_DOUBLE,
} ArithKind;

typedef struct ArithType {
    ArithKind arithKind;
} ArithType;

typedef struct PointerType {
    QualType pointee;
} PointerType;

typedef struct ConstantArrayType {

} ConstantArrayType;

typedef struct VariableArrayType {

} VariableArrayType;

typedef enum ArrayKind {
    ARRAY_CONSTANT,
    ARRAY_VARIABLE,
} ArrayKind;

typedef struct ArrayType {
    ArrayKind arrKind;
    QualType elemType;
    union {
        ConstantArrayType cArrayTy;
        VariableArrayType vArrayTy;
    } as;
} ArrayType;

typedef struct FunctionType {

} FunctionType;

typedef struct RecordType {

} RecordType;

typedef struct EnumType {

} EnumType;

typedef struct TypedefType {

} TypedefType;

typedef enum TypeKind {
    TYPE_VOID,
    TYPE_ARITH,
    TYPE_POINTER,
    TYPE_ARRAY,
    TYPE_FUNCTION,
    TYPE_RECORD,
    TYPE_ENUM,
    TYPE_TYPEDEF
} TypeKind;

typedef struct Type {
    QualType canonicalType;
    TypeKind kind;
    union {
        VoidType voidTy;
        ArithType arithTy;
        PointerType pointerTy;
        ArrayType arrayTy;
        FunctionType functionTy;
        RecordType recordTy;
        EnumType enumTy;
        TypedefType typedefTy;
    } as;
} Type;

#endif