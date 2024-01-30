/**
 * https://www.quut.com/c/ANSI-C-grammar-y-1999.html
 */

#ifndef _CRYOLITE_SYNTAX_H_
#define _CRYOLITE_SYNTAX_H_

#include <optional>
#include <variant>
#include <vector>

class Expr;

// PostfixExpression
class PostfixExprSubscript;
class PostfixExprDot;
class PostfixExprFuncall;
class PostfixExprArrow;
class PostfixExprIncrement;
class PostfixExprDecrement;
class PostfixExprTypeInitializer;

// UnaryExpression
class UnaryExprIncrement;
class UnaryExprDecrement;
class UnaryExprUnaryOperator;
class UnaryExprSizeOf;

class TypeName;
class CastExpr;
class AssignmentExpr;

class Initializer;
class InitDeclarator;
class DeclSpecifiers;
class Declarator;
class InitializerList;
class StorageClassSpecifier;
class TypeSpecifier;
class TypeQualifier;
class FuncSpecifier;
class StructOrUnionSpecifier;
class EnumSpecifier;
class Pointer;
class ParamTypeList;
class ParamDeclaration;
class AbstractDeclarator;
class DirectAbstractDeclarator;

// DirectDeclarator
class DirectDeclarator;
class DirectDeclaratorIdentifier;
class DirectDeclaratorParentheses;
class DirectDeclaratorNoStaticOrAsterisk;
class DirectDeclaratorStatic;
class DirectDeclaratorAsterisk;
class DirectDeclaratorParamTypeList;

// DirectAbstractDeclarator
class DirectAbstractDeclaratorParentheses;
class DirectAbstractDeclaratorAssignExpr;
class DirectAbstractDeclaratorAsterisk;
class DirectAbstractDeclaratorParamTypeList;

// Statement
class LabelStmt;
class CaseStmt;
class DefaultStmt;
class BlockStmt;
class ExprStmt;
class IfStmt;
class SwitchStmt;
class WhileStmt;
class DoWhileStmt;
class ForStmt;
class GotoStmt;
class ContinueStmt;
class BreakStmt;
class ReturnStmt;

/*
 * primary_expression:
 *     IDENTIFIER
 */
class PrimaryExprIdentifier {
public:
    std::string ident;
};

/*
 * primary_expression:
 *     CONSTANT
 *     STRING_LITERAL
 */
class PrimaryExprConstant {
public:
    std::variant<int32_t,
                 uint32_t,
                 int64_t,
                 uint64_t,
                 float,
                 double,
                 std::string>
        value;
};

/*
 * primary_expression:
 *     ( expression )
 */
class PrimaryExprParentheses {
public:
    Expr *expr;
};

/*
 * primary_expression:
 *     IDENTIFIER
 *     CONSTANT
 *     STRING_LITERAL
 *     ( expression )
 */
class PrimaryExpr {
public:
    std::variant<PrimaryExprIdentifier *,
                 PrimaryExprConstant *,
                 PrimaryExprParentheses *>
        var;
};

/**
 * postfix_expression:
 *     primary_expression
 *     postfix_expression [ expression ]
 *     postfix_expression ( )
 *     postfix_expression ( argument_expression_list )
 *     postfix_expression . IDENTIFIER
 *     postfix_expression -> IDENTIFIER
 *     postfix_expression ++
 *     postfix_expression --
 *     ( type_name ) { initializer_list }
 *     ( type_name ) { initializer_list , }
 */
class PostfixExpr {
public:
    std::variant<PrimaryExpr *,
                 PostfixExprSubscript *,
                 PostfixExprFuncall *,
                 PostfixExprDot *,
                 PostfixExprArrow *,
                 PostfixExprIncrement *,
                 PostfixExprDecrement *,
                 PostfixExprTypeInitializer *>
        var;
};

/**
 * postfix_expression:
 *     postfix_expression [ expression ]
 */
class PostfixExprSubscript {
public:
    PostfixExpr *postfixExpr;
    Expr *expr;
};

/**
 * postfix_expression:
 *     postfix_expression ( )
 *     postfix_expression ( argument_expression_list )
 *
 * argument_expression_list:
 *     assignment_expression
 *     argument_expression_list , assignment_expression
 */
class PostfixExprFuncall {
public:
    PostfixExpr *postfixExpr;
    std::vector<AssignmentExpr *> args;
};

/**
 * postfix_expression:
 *     postfix_expression . IDENTIFIER
 */
class PostfixExprDot {
public:
    PostfixExpr *postfixExpr;
    std::string ident;
};

/**
 * postfix_expression:
 *     postfix_expression -> IDENTIFIER
 */
class PostfixExprArrow {
public:
    PostfixExpr *postfixExpr;
    std::string ident;
};

/**
 * postfix_expression:
 *     postfix_expression ++
 */
class PostfixExprIncrement {
public:
    PostfixExpr *postfixExpr;
};

/**
 * postfix_expression:
 *     postfix_expression --
 */
class PostfixExprDecrement {
public:
    PostfixExpr *postfixExpr;
};

/**
 * postfix_expression:
 *     ( type_name ) { initializer_list }
 *     ( type_name ) { initializer_list , }
 */
class PostfixExprTypeInitializer {
public:
    TypeName *typeName;
    InitializerList *initList;
};

/**
 * unary_expression:
 *     postfix_expression
 *     ++ unary_expression
 *     -- unary_expression
 *     unary_operator cast_expression
 *     SIZEOF unary_expression
 *     SIZEOF ( type_name )
 */
class UnaryExpr {
public:
    std::variant<PostfixExpr *,
                 UnaryExprIncrement *,
                 UnaryExprDecrement *,
                 UnaryExprUnaryOperator *,
                 UnaryExprSizeOf *>
        var;
};

/**
 * unary_expression:
 *     ++ unary_expression
 */
class UnaryExprIncrement {
public:
    UnaryExpr *unaryExpr;
};

/**
 * unary_expression:
 *     -- unary_expression
 */
class UnaryExprDecrement {
public:
    UnaryExpr *unaryExpr;
};

/**
 * unary_operator:
 *     &
 *     *
 *     +
 *     -
 *     ~
 *     !
 */
enum UnaryOp {
    AMP,
    STAR,
    PLUS,
    MINUS,
    TILDE,
    EXCLAIM
};

/**
 * unary_expression:
 *     unary_operator cast_expression
 */
class UnaryExprUnaryOperator {
public:
    UnaryOp unaryOp;
    CastExpr *castExpr;
};

/**
 * unary_expression:
 *     SIZEOF unary_expression
 *     SIZEOF ( type_name )
 */
class UnaryExprSizeOf {
public:
    std::variant<UnaryExpr *, TypeName *> var;
};

/**
 * cast_expression:
 *     unary_expression
 *     ( type_name ) cast_expression
 */
class CastExpr {
public:
    std::vector<TypeName *> typeNames;
    UnaryExpr *unaryExpr;
};

/**
 * multiplicative_expression:
 *     cast_expression
 *     multiplicative_expression * cast_expression
 *     multiplicative_expression / cast_expression
 *     multiplicative_expression % cast_expression
 */
class MultiplicativeExpr {
public:
    enum MEOp {
        STAR,
        SLASH,
        PERCENT
    };
    CastExpr *castExpr;
    std::vector<std::pair<MEOp, CastExpr *>> exprs;
};

/**
 * additive_expression:
 *     multiplicative_expression
 *     additive_expression + multiplicative_expression
 *     additive_expression - multiplicative_expression
 */
class AdditiveExpr {
public:
    enum AEOp {
        PLUS,
        MINUS
    };
    MultiplicativeExpr *multiExpr;
    std::vector<std::pair<AEOp, MultiplicativeExpr *>> exprs;
};

/**
 * shift_expression:
 *     additive_expression
 *     shift_expression << additive_expression
 *     shift_expression >> additive_expression
 */
class ShiftExpr {
public:
    enum SEOp {
        LESSLESS,
        GREATERGREATER
    };
    AdditiveExpr *addiExpr;
    std::vector<std::pair<SEOp, AdditiveExpr *>> exprs;
};

/**
 * relational_expression:
 *     shift_expression
 *     relational_expression < shift_expression
 *     relational_expression > shift_expression
 *     relational_expression <= shift_expression
 *     relational_expression >= shift_expression
 */
class RelationalExpr {
public:
    enum REOp {
        LESS,
        GREATER,
        LESSEQUAL,
        GREATEREQUAL
    };
    ShiftExpr *shiftExpr;
    std::vector<std::pair<REOp, ShiftExpr *>> exprs;
};

/**
 * equality_expression:
 *     relational_expression
 *     equality_expression == relational_expression
 *     equality_expression != relational_expression
 */
class EqualityExpr {
public:
    enum EEOp {
        EQUALEQUAL,
        EXCLAIMEQUAL
    };
    RelationalExpr *relatExpr;
    std::vector<std::pair<EEOp, RelationalExpr *>> exprs;
};

/**
 * and_expression:
 *     equality_expression
 *     and_expression & equality_expression
 */
class BitAndExpr {
public:
    std::vector<EqualityExpr *> exprs;
};

/**
 * exclusive_or_expression:
 *     and_expression
 *     exclusive_or_expression ^ and_expression
 */
class BitXorExpr {
public:
    std::vector<BitAndExpr *> exprs;
};

/**
 * inclusive_or_expression:
 *      exclusive_or_expression
 *      inclusive_or_expression | exclusive_or_expression
 */
class BitOrExpr {
public:
    std::vector<BitXorExpr *> exprs;
};

/**
 * logical_and_expression:
 *      inclusive_or_expression
 *      logical_and_expression && inclusive_or_expression
 */
class LogicalAndExpr {
public:
    std::vector<BitOrExpr *> exprs;
};

/**
 * logical_or_expression:
 *      logical_and_expression
 *      logical_or_expression || logical_and_expression
 */
class LogicalOrExpr {
public:
    std::vector<LogicalAndExpr *> exprs;
};

/**
 * conditional_expression:
 *      logical_or_expression
 *      logical_or_expression ? expression : conditional_expression
 */
class ConditionalExpr {
public:
    LogicalOrExpr *logiOrExpr;
    std::optional<Expr *> expr;
    std::optional<ConditionalExpr *> condExpr;
};

/**
 * assignment_operator:
 *     =
 *     *=
 *     /=
 *     %=
 *     +=
 *     -=
 *     <<=
 *     >>=
 *     &=
 *     ^=
 *     |=
 */
enum AssignOp {
    EQUAL,               // =
    STAREQUAL,           // *=
    SLASHEQUAL,          // /=
    PERCENTEQUAL,        // %=
    PLUSEQUAL,           // +=
    MINUSEQUAL,          // -=
    LESSLESSEQUAL,       // <<=
    GREATERGREATEREQUAL, // >>=
    AMPEQUAL,            // &=
    CARETEQUAL,          // ^=
    PIPEEQUAL            // |=
};

/**
 * According to C99:
 *
 * assignment_expression:
 *     conditional_expression
 *     unary_expression assignment_operator assignment_expression
 *
 * Instead we are doing something similar to clang here though.
 * We'll be using the grammar of the form:
 *
 * assignment_expression:
 *     conditional_expression
 *     conditional_expression assignment_operator assignment_expression
 *
 * Checking if the left operand is an LValue will be done in semantics analysis.
 */
class AssignmentExpr {
public:
    ConditionalExpr *condExpr;
    std::vector<std::pair<AssignOp, ConditionalExpr *>> exprs;
};

/**
 * expression:
 *     assignment_expression
 *     expression , assignment_expression
 */
class Expr {
public:
    std::vector<AssignmentExpr *> exprs;
};

/**
 * constant_expression:
 *     conditional_expression
 */
class ConstantExpr {
public:
    ConditionalExpr *condExpr;
};

/**
 * declaration:
 *     declaration_specifiers ;
 *     declaration_specifiers init_declarator_list ;
 *
 * init_declarator_list:
 *     init_declarator
 *     init_declarator_list , init_declarator
 */
class Declaration {
public:
    Declaration(DeclSpecifiers *declSpecs)
        : declSpecs(declSpecs) {}

    DeclSpecifiers *declSpecs;
    std::vector<InitDeclarator *> initDecls;
};

/**
 * declaration_specifiers:
 *     storage_class_specifier
 *     storage_class_specifier declaration_specifiers
 *     type_specifier
 *     type_specifier declaration_specifiers
 *     type_qualifier
 *     type_qualifier declaration_specifiers
 *     function_specifier
 *     function_specifier declaration_specifiers
 */
class DeclSpecifier {
public:
    DeclSpecifier(StorageClassSpecifier *storageClassSpec)
        : var(storageClassSpec) {}
    DeclSpecifier(TypeSpecifier *typeSpec)
        : var(typeSpec) {}
    DeclSpecifier(TypeQualifier *typeQual)
        : var(typeQual) {}
    DeclSpecifier(FuncSpecifier *funcSpec)
        : var(funcSpec) {}

    std::variant<StorageClassSpecifier *,
                 TypeSpecifier *,
                 TypeQualifier *,
                 FuncSpecifier *>
        var;
};

class DeclSpecifiers {
public:
    void AddStorageClassSpecifier(StorageClassSpecifier *storageClassSpec);
    void AddTypeSpecifier(TypeSpecifier *typeSpec);
    void AddTypeQualifier(TypeQualifier *typeQual);
    void AddFuncSpecifier(FuncSpecifier *funcSpec);

    std::vector<DeclSpecifier *> specs;
};

/**
 * init_declarator:
 *     declarator
 *     declarator = initializer
 */
class InitDeclarator {
public:
    Declarator *declarator;
    std::optional<Initializer *> initializer;
};

/**
 * storage_class_specifier:
 *     TYPEDEF
 *     EXTERN
 *     STATIC
 *     AUTO
 *     REGISTER
 */
class StorageClassSpecifier {
public:
    enum Specifier {
        TYPEDEF,
        EXTERN,
        STATIC,
        AUTO,
        REGISTER
    };

    StorageClassSpecifier(Specifier spec)
        : spec(spec) {}

    Specifier spec;
};

/**
 * type_specifier:
 *     VOID
 *     CHAR
 *     SHORT
 *     INT
 *     LONG
 *     FLOAT
 *     DOUBLE
 *     SIGNED
 *     UNSIGNED
 *     struct_or_union_specifier
 *     enum_specifier
 *     TYPE_NAME
 *
 * Where TYPE_NAME is also known as IDENTIFIER.
 */
class TypeSpecifier {
public:
    enum PrimTypeKind {
        VOID,
        CHAR,
        SHORT,
        INT,
        LONG,
        FLOAT,
        DOUBLE,
        SIGNED,
        UNSIGNED
    };

    TypeSpecifier(PrimTypeKind typeKind)
        : var(typeKind) {}
    TypeSpecifier(StructOrUnionSpecifier *sOrUSpec)
        : var(sOrUSpec) {}
    TypeSpecifier(EnumSpecifier *enumSpec)
        : var(enumSpec) {}
    TypeSpecifier(std::string ident)
        : var(ident) {}

    std::variant<PrimTypeKind,
                 StructOrUnionSpecifier *,
                 EnumSpecifier *,
                 std::string>
        var;
};

/**
 * struct_declarator:
 *     declarator
 *     : constant_expression
 *     declarator : constant_expression
 */
class StructDeclarator {
public:
    std::optional<Declarator *> declarator;
    std::optional<ConstantExpr *> constExpr;
};

/**
 * specifier_qualifier_list:
 *     type_specifier specifier_qualifier_list
 *     type_specifier
 *     type_qualifier specifier_qualifier_list
 *     type_qualifier
 */
class SpecifierQualifier {
public:
    std::variant<TypeSpecifier *, TypeQualifier *> var;
};

/**
 * struct_declaration:
 *     specifier_qualifier_list struct_declarator_list ;
 *
 * struct_declarator_list:
 *     struct_declarator
 *     struct_declarator_list , struct_declarator
 */
class StructDeclaration {
public:
    std::vector<SpecifierQualifier *> specQualList;
    std::vector<StructDeclarator *> structDeclarators;
};

/**
 * struct_or_union_specifier:
 *     struct_or_union IDENTIFIER { struct_declaration_list }
 *     struct_or_union { struct_declaration_list }
 *     struct_or_union IDENTIFIER
 *
 * struct_or_union:
 *     STRUCT
 *     UNION
 *
 * struct_declaration_list:
 *     struct_declaration
 *     struct_declaration_list struct_declaration
 */
class StructOrUnionSpecifier {
public:
    bool isUnion;
    std::string ident;
    std::vector<StructDeclaration *> structDeclList;
};

/**
 * enumerator:
 *     IDENTIFIER
 *     IDENTIFIER = constant_expression
 */
class Enumerator {
public:
    std::string ident;
    std::optional<ConstantExpr *> constExpr;
};

/**
 * enum_specifier:
 *     ENUM { enumerator_list }
 *     ENUM IDENTIFIER { enumerator_list }
 *     ENUM { enumerator_list , }
 *     ENUM IDENTIFIER { enumerator_list , }
 *     ENUM IDENTIFIER
 *
 * enumerator_list:
 *     enumerator
 *     enumerator_list , enumerator
 */
class EnumSpecifier {
public:
    std::string ident;
    std::vector<Enumerator *> enumerators;
};

/**
 * type_qualifier:
 *     CONST
 *     RESTRICT
 *     VOLATILE
 */
class TypeQualifier {
public:
    enum TypeQual {
        CONST,
        RESTRICT,
        VOLATILE
    };

    TypeQualifier(TypeQual typeQual)
        : typeQual(typeQual) {}

    TypeQual typeQual;
};

/**
 * function_specifier:
 *     INLINE
 */
class FuncSpecifier {
public:
    enum FuncSpec {
        INLINE
    };

    FuncSpecifier(FuncSpec funcSpec)
        : funcSpec(funcSpec) {}

    FuncSpec funcSpec;
};

/**
 * declarator:
 *     pointer direct_declarator
 *     direct_declarator
 */
class Declarator {
public:
    std::vector<Pointer *> pointers;
    DirectDeclarator *directDeclarator;
};

/**
 * direct_declarator:
 *     IDENTIFIER
 *     ( declarator )
 *     direct_declarator [ type_qualifier_list assignment_expression ]
 *     direct_declarator [ type_qualifier_list ]
 *     direct_declarator [ assignment_expression ]
 *     direct_declarator [ ]
 *     direct_declarator [ STATIC type_qualifier_list assignment_expression ]
 *     direct_declarator [ type_qualifier_list STATIC assignment_expression ]
 *     direct_declarator [ type_qualifier_list * ]
 *     direct_declarator [ * ]
 *     direct_declarator ( parameter_type_list )
 *     direct_declarator ( identifier_list )
 *     direct_declarator ( )
 *
 * "direct_declarator ( identifier_list )" here is used to support old-style declarator,
 * which we don't support. e.g.:
 *     int old(a, b) int a; int b; { return a + b; }
 */
class DirectDeclarator {
public:
    std::variant<DirectDeclaratorIdentifier *,
                 DirectDeclaratorParentheses *,
                 DirectDeclaratorNoStaticOrAsterisk *,
                 DirectDeclaratorStatic *,
                 DirectDeclaratorAsterisk *,
                 DirectDeclaratorParamTypeList *>
        var;
};

/**
 * direct_declarator:
 *     IDENTIFIER
 */
class DirectDeclaratorIdentifier {
public:
    std::string ident;
};

/**
 * direct_declarator:
 *     ( declarator )
 */
class DirectDeclaratorParentheses {
public:
    Declarator *declarator;
};

/**
 * direct_declarator:
 *     direct_declarator [ type_qualifier_list assignment_expression ]
 *     direct_declarator [ type_qualifier_list ]
 *     direct_declarator [ assignment_expression ]
 *     direct_declarator [ ]
 */
class DirectDeclaratorNoStaticOrAsterisk {
public:
    DirectDeclarator *directDeclarator;
    std::vector<TypeQualifier *> typeQuals;
    std::optional<AssignmentExpr *> assignExpr;
};

/**
 * direct_declarator:
 *     direct_declarator [ STATIC type_qualifier_list assignment_expression ]
 *     direct_declarator [ type_qualifier_list STATIC assignment_expression ]
 */
class DirectDeclaratorStatic {
public:
    DirectDeclarator *directDeclarator;
    bool staticFirst;
    std::vector<TypeQualifier *> typeQuals;
    AssignmentExpr *assignExpr;
};

/**
 * direct_declarator:
 *     direct_declarator [ type_qualifier_list * ]
 *     direct_declarator [ * ]
 */
class DirectDeclaratorAsterisk {
public:
    DirectDeclarator *directDeclarator;
    std::vector<TypeQualifier *> typeQuals;
};

/**
 * direct_declarator:
 *     direct_declarator ( parameter_type_list )
 */
class DirectDeclaratorParamTypeList {
public:
    DirectDeclarator *directDeclarator;
    ParamTypeList *paramTypeList;
};

/**
 * pointer:
 *     *
 *     * type_qualifier_list
 *     * pointer
 *     * type_qualifier_list pointer
 */
class Pointer {
public:
    std::vector<TypeQualifier *> typeQuals;
};

/**
 * parameter_type_list:
 *     parameter_list
 *     parameter_list , ...
 *
 * parameter_list:
 *     parameter_declaration
 *     parameter_list , parameter_declaration
 */
class ParamTypeList {
public:
    std::vector<ParamDeclaration *> paramList;
    bool hasEllipsis;
};

/**
 * parameter_declaration:
 *     declaration_specifiers declarator
 *     declaration_specifiers abstract_declarator
 *     declaration_specifiers
 */
class ParamDeclaration {
public:
    DeclSpecifiers *declSpecs;
    std::variant<Declarator *, std::optional<AbstractDeclarator *>> var;
};

/**
 * type_name:
 *     specifier_qualifier_list abstract_declarator
 */
class TypeName {
public:
    std::vector<SpecifierQualifier *> specQualList;
    std::optional<AbstractDeclarator *> absDeclarator;
};

/**
 * abstract_declarator:
 *     pointer
 *     pointer direct_abstract_declarator
 *     direct_abstract_declarator
 */
class AbstractDeclarator {
public:
    std::vector<Pointer *> pointers;
    std::optional<DirectAbstractDeclarator *> directAbsDeclarator;
};

/**
 * direct_abstract_declarator:
 *     ( abstract_declarator )
 *     direct_abstract_declarator [ assignment_expression ]
 *     direct_abstract_declarator [ ]
 *     [ assignment_expression ]
 *     [ ]
 *     direct_abstract_declarator [ * ]
 *     [ * ]
 *     direct_abstract_declarator ( parameter_type_list )
 *     direct_abstract_declarator ( )
 *     ( parameter_type_list )
 *     ( )
 */
class DirectAbstractDeclarator {
public:
    std::variant<DirectAbstractDeclaratorParentheses *,
                 DirectAbstractDeclaratorAssignExpr *,
                 DirectAbstractDeclaratorAsterisk *,
                 DirectAbstractDeclaratorParamTypeList *>
        var;
};

/**
 * direct_abstract_declarator:
 *     ( abstract_declarator )
 */
class DirectAbstractDeclaratorParentheses {
public:
    AbstractDeclarator *absDeclarator;
};

/**
 * direct_abstract_declarator:
 *     direct_abstract_declarator [ assignment_expression ]
 *     direct_abstract_declarator [ ]
 *     [ assignment_expression ]
 *     [ ]
 */
class DirectAbstractDeclaratorAssignExpr {
public:
    std::optional<DirectAbstractDeclarator *> directAbsDeclarator;
    std::optional<AssignmentExpr *> assignExpr;
};

/**
 * direct_abstract_declarator:
 *     direct_abstract_declarator [ * ]
 *     [ * ]
 */
class DirectAbstractDeclaratorAsterisk {
public:
    std::optional<DirectAbstractDeclarator *> directAbsDeclarator;
};

/**
 * direct_abstract_declarator:
 *     direct_abstract_declarator ( parameter_type_list )
 *     direct_abstract_declarator ( )
 *     ( parameter_type_list )
 *     ( )
 */
class DirectAbstractDeclaratorParamTypeList {
public:
    std::optional<DirectAbstractDeclarator *> directAbsDeclarator;
    std::optional<ParamTypeList *> paramTypeList;
};

/**
 * initializer:
 *     assignment_expression
 *     { initializer_list }
 *     { initializer_list , }
 */
class Initializer {
public:
    std::variant<AssignmentExpr *, InitializerList *> var;
};

/**
 * designator:
 *     [ constant_expression ]
 *     . IDENTIFIER
 */
class Designator {
public:
    std::variant<ConstantExpr *, std::string> var;
};

/**
 * initializer_list:
 *     designation initializer
 *     initializer
 *     initializer_list , designation initializer
 *     initializer_list , initializer
 */
class InitializerPair {
public:
    std::optional<std::vector<Designator *>> designation;
    Initializer *initializer;
};

/**
 * initializer_list:
 *     designation initializer
 *     initializer
 *     initializer_list , designation initializer
 *     initializer_list , initializer
 *
 * designation:
 *     designator_list =
 *
 * designator_list:
 *     designator
 *     designator_list designator
 *
 * designator:
 *     [ constant_expression ]
 *     . IDENTIFIER
 *
 * e.g.:
 *     struct { int a[3], b; } w[] = { [0].a = {1}, [1].a[0] = 2 };
 * [0].a meaning designator designator
 */
class InitializerList {
public:
    std::vector<InitializerPair *> initializerPairs;
};

/**
 * statement:
 *     labeled_statement
 *     compound_statement
 *     expression_statement
 *     selection_statement
 *     iteration_statement
 *     jump_statement
 */
class Stmt {
public:
    std::variant<LabelStmt *,
                 CaseStmt *,
                 DefaultStmt *,
                 BlockStmt *,
                 ExprStmt *,
                 IfStmt *,
                 SwitchStmt *,
                 WhileStmt *,
                 DoWhileStmt *,
                 ForStmt *,
                 GotoStmt *,
                 ContinueStmt *,
                 BreakStmt *,
                 ReturnStmt *>
        var;
};

/**
 * labeled_statement:
 *     IDENTIFIER : statement
 *     CASE constant_expression : statement
 *     DEFAULT : statement
 */
class LabelStmt {
public:
    std::string ident;
    Stmt *stmt;
};

class CaseStmt {
public:
    ConstantExpr *constExpr;
    Stmt *stmt;
};

class DefaultStmt {
public:
    Stmt *stmt;
};

/**
 * block_item:
 *     declaration
 *     statement
 */
class BlockItem {
public:
    std::variant<Stmt *, Declaration *> var;
};

/**
 * compound_statement:
 *     { }
 *     { block_item_list }
 *
 * block_item_list:
 *     block_item
 *     block_item_list block_item
 */
class BlockStmt {
public:
    std::vector<BlockItem *> blockItemList;
};

/**
 * expression_statement:
 *     ;
 *     expression ;
 */
class ExprStmt {
public:
    std::optional<Expr *> expr;
};

/**
 * selection_statement:
 *     IF ( expression ) statement
 *     IF ( expression ) statement ELSE statement
 *     SWITCH ( expression ) statement
 */
class IfStmt {
public:
    Expr *expr;
    Stmt *thenStmt;
    std::optional<Stmt *> elseStmt;
};

class SwitchStmt {
public:
    Expr *expr;
    Stmt *stmt;
};

/**
 * iteration_statement:
 *     WHILE ( expression ) statement
 *     DO statement WHILE ( expression ) ;
 *     FOR ( expression_statement expression_statement ) statement
 *     FOR ( expression_statement expression_statement expression ) statement
 *     FOR ( declaration expression_statement ) statement
 *     FOR ( declaration expression_statement expression ) statement
 */
class WhileStmt {
public:
    Expr *expr;
    Stmt *stmt;
};

class DoWhileStmt {
public:
    Stmt *stmt;
    Expr *expr;
};

class ForStmt {
public:
    std::variant<ExprStmt *, Declaration *> init;
    ExprStmt *exprStmt;
    std::optional<Expr *> expr;
    Stmt *stmt;
};

/**
 * jump_statement:
 *     GOTO IDENTIFIER ;
 *     CONTINUE ;
 *     BREAK ;
 *     RETURN ;
 *     RETURN expression ;
 */
class GotoStmt {
public:
    std::string ident;
};

class ContinueStmt {
public:
};

class BreakStmt {
public:
};

class ReturnStmt {
public:
    std::optional<Expr *> expr;
};

/**
 * function_definition:
 *     declaration_specifiers declarator declaration_list compound_statement
 *     declaration_specifiers declarator compound_statement
 *
 * "declaration_list" here is used to support old-style function definitions, which we don't support.
 * e.g.:
 *     int old(a, b) int a; int b; { return a + b; }
 */
class FunctionDefinition {
public:
    DeclSpecifiers *declSpecs;
    Declarator *declarator;
    BlockStmt *compoundStmt;
};

/**
 * external_declaration:
 *     function_definition
 *     declaration
 */
class ExternalDeclaration {
public:
    ExternalDeclaration() = default;
    ExternalDeclaration(Declaration *decl)
        : var(decl) {}
    ExternalDeclaration(FunctionDefinition *funcDef)
        : var(funcDef) {}

    std::variant<Declaration *, FunctionDefinition *> var;
};

/**
 * translation_unit:
 *     external_declaration
 *     translation_unit external_declaration
 *
 * The entry of the syntax tree.
 */
class TranslationUnit {
public:
    std::vector<ExternalDeclaration *> extDecls;
};

#endif