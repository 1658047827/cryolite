#ifndef _CRYOLITE_TYPE_H_
#define _CRYOLITE_TYPE_H_

struct Type;

typedef enum Qualifier {
    QUAL_CONST = 1 << 0,
    QUAL_RESTRICT = 1 << 1,
    QUAL_VOLATILE = 1 << 2,
} Qualifier;

typedef struct QualType {
    unsigned char quals;
    struct Type *t;
} QualType;

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
    TypeKind kind;
    QualType canonicalType;
} Type;

typedef struct VoidType {
    Type type;
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
    Type type;
    ArithKind arithKind;
} ArithType;

typedef struct PointerType {
    Type type;
    QualType pointee;
} PointerType;

typedef enum ArrayKind {
    ARRAY_CONSTANT,
    ARRAY_VARIABLE,
} ArrayKind;

typedef struct ArrayType {
    Type type;
    ArrayKind arrKind;
    QualType elemType;
} ArrayType;

typedef struct ConstantArrayType {
    ArrayType arrayType;
} ConstantArrayType;

typedef struct VariableArrayType {
    ArrayType arrayType;
} VariableArrayType;

typedef struct FunctionType {
    Type type;
    QualType retType;
} FunctionType;

typedef struct RecordType {
    Type type;
} RecordType;

typedef struct EnumType {
    Type type;
} EnumType;

typedef struct TypedefType {
    Type type;
} TypedefType;

extern QualType voidTy;
extern QualType boolTy;
extern QualType charTy;
extern QualType signedCharTy, shortTy, intTy, longTy, longLongTy;
extern QualType unsignedCharTy, unsignedShortTy, unsignedIntTy, unsignedLongTy, unsignedLongLongTy;
extern QualType floatTy, doubleTy, longDoubleTy;

VoidType *newVoidType();
ArithType *newArithType(ArithKind k);

#endif