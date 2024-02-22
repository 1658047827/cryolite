/**
 * syntax.h - C99 syntax definitions.
 *
 * This header file defines the syntax rules for the C99 standard, based on the
 * official C99 specification document https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1256.pdf
 * and the yacc grammar from the website https://www.quut.com/c/ANSI-C-grammar-y-1999.html
 */

#ifndef _CRYOLITE_SYNTAX_H_
#define _CRYOLITE_SYNTAX_H_

#include <optional>
#include <string>
#include <variant>
#include <vector>

/**
 * Expressions
 */
class PrimaryExpression;
class PostfixExpression;
class UnaryExpression;
class CastExpression;
class MultiplicativeExpression;
class AdditiveExpression;
class ShiftExpression;
class RelationalExpression;
class EqualityExpression;
class BitAndExpression;
class BitXorExpression;
class BitOrExpression;
class LogicalAndExpression;
class LogicalOrExpression;
class ConditionalExpression;
class AssignmentExpression;
class Expression;

/**
 * Constant expressions
 */
class ConstantExpression;

/**
 * Declarations
 */
class Declaration;
class DeclarationSpecifiers;
class InitDeclarator;
class StorageClassSpecifier;
class TypeSpecifier;
class StructOrUnionSpecifier;
class StructDeclaration;
class SpecifierQualifier;
class StructDeclarator;
class EnumSpecifier;
class Enumerator;
class TypeQualifier;
class FunctionSpecifier;
class Declarator;
class DirectDeclarator;
class Pointer;
class ParameterTypeList;
class ParameterDeclaration;
class TypeName;
class AbstractDeclarator;
class DirectAbstractDeclarator;
class Initializer;
class InitializerList;
class Designator;

/**
 * Statements and blocks
 */
class Statement;
class LabelStatement;
class CaseStatement;
class DefaultStatement;
class CompoundStatement;
class BlockItem;
class ExpressionStatement;
class IfStatement;
class SwitchStatement;
class WhileStatement;
class DoWhileStatement;
class ForStatement;
class GotoStatement;
class ContinueStatement;
class BreakStatement;
class ReturnStatement;

/**
 * External definitions
 */
class TranslationUnit;
class ExternalDeclaration;
class FunctionDefinition;

/*
 * primary-expression:
 *     IDENTIFIER
 */
class PrimaryExpressionIdentifier {
public:
    std::string ident;
};

/*
 * primary-expression:
 *     CONSTANT
 *     STRING_LITERAL
 */
class PrimaryExpressionConstant {
public:
    std::variant<int8_t, uint8_t,
                 int16_t, uint16_t,
                 int32_t, uint32_t,
                 int64_t, uint64_t,
                 float, double,
                 std::string>
        value;
};

/*
 * primary-expression:
 *     ( expression )
 */
class PrimaryExpressionParentheses {
public:
    Expression *expr;
};

/*
 * primary-expression:
 *     IDENTIFIER
 *     CONSTANT
 *     STRING_LITERAL
 *     ( expression )
 */
class PrimaryExpression {
public:
    std::variant<PrimaryExpressionIdentifier *,
                 PrimaryExpressionConstant *,
                 PrimaryExpressionParentheses *>
        var;
};

/**
 * postfix-expression:
 *     postfix-expression [ expression ]
 */
class PostfixExpressionArraySubscript {
public:
    PostfixExpression *postfixExpr;
    Expression *expr;
};

/**
 * postfix-expression:
 *     postfix-expression ( argument-expression-list{opt} )
 *
 * argument-expression-list:
 *     assignment-expression
 *     argument-expression-list , assignment-expression
 */
class PostfixExpressionFunctionCall {
public:
    PostfixExpression *postfixExpr;
    std::vector<AssignmentExpression *> args;
};

/**
 * postfix-expression:
 *     postfix-expression . IDENTIFIER
 */
class PostfixExpressionDotMember {
public:
    PostfixExpression *postfixExpr;
    std::string ident;
};

/**
 * postfix-expression:
 *     postfix-expression -> IDENTIFIER
 */
class PostfixExpressionArrowMember {
public:
    PostfixExpression *postfixExpr;
    std::string ident;
};

/**
 * postfix-expression:
 *     postfix-expression ++
 */
class PostfixExpressionIncrement {
public:
    PostfixExpression *postfixExpr;
};

/**
 * postfix-expression:
 *     postfix-expression --
 */
class PostfixExpressionDecrement {
public:
    PostfixExpression *postfixExpr;
};

/**
 * postfix-expression:
 *     ( type-name ) { initializer-list }
 *     ( type-name ) { initializer-list , }
 */
class PostfixExpressionCompoundLiteral {
public:
    TypeName *typeName;
    InitializerList *initList;
};

/**
 * postfix-expression:
 *     primary-expression
 *     postfix-expression [ expression ]
 *     postfix-expression ( argument-expression-list{opt} )
 *     postfix-expression . IDENTIFIER
 *     postfix-expression -> IDENTIFIER
 *     postfix-expression ++
 *     postfix-expression --
 *     ( type-name ) { initializer-list }
 *     ( type-name ) { initializer-list , }
 */
class PostfixExpression {
public:
    std::variant<PrimaryExpression *,
                 PostfixExpressionArraySubscript *,
                 PostfixExpressionFunctionCall *,
                 PostfixExpressionDotMember *,
                 PostfixExpressionArrowMember *,
                 PostfixExpressionIncrement *,
                 PostfixExpressionDecrement *,
                 PostfixExpressionCompoundLiteral *>
        var;
};

/**
 * unary-expression:
 *     ++ unary-expression
 */
class UnaryExpressionIncrement {
public:
    UnaryExpression *unaryExpr;
};

/**
 * unary-expression:
 *     -- unary-expression
 */
class UnaryExpressionDecrement {
public:
    UnaryExpression *unaryExpr;
};

/**
 * unary-operator:
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
 * unary-expression:
 *     unary-operator cast-expression
 */
class UnaryExpressionUnaryOperator {
public:
    UnaryOp unaryOp;
    CastExpression *castExpr;
};

/**
 * unary-expression:
 *     SIZEOF unary-expression
 *     SIZEOF ( type-name )
 */
class UnaryExpressionSizeOf {
public:
    std::variant<UnaryExpression *, TypeName *> var;
};

/**
 * unary-expression:
 *     postfix-expression
 *     ++ unary-expression
 *     -- unary-expression
 *     unary-operator cast-expression
 *     SIZEOF unary-expression
 *     SIZEOF ( type-name )
 */
class UnaryExpression {
public:
    std::variant<PostfixExpression *,
                 UnaryExpressionIncrement *,
                 UnaryExpressionDecrement *,
                 UnaryExpressionUnaryOperator *,
                 UnaryExpressionSizeOf *>
        var;
};

/**
 * cast-expression:
 *     unary-expression
 *     ( type-name ) cast-expression
 */
class CastExpression {
public:
    std::variant<UnaryExpression *, std::pair<TypeName *, CastExpression *>> var;
};

/**
 * multiplicative-expression:
 *     cast-expression
 *     multiplicative-expression * cast-expression
 *     multiplicative-expression / cast-expression
 *     multiplicative-expression % cast-expression
 */
class MultiplicativeExpression {
public:
    enum MEOp {
        STAR,
        SLASH,
        PERCENT
    };
    CastExpression *castExpr;
    std::vector<std::pair<MEOp, CastExpression *>> exprs;
};

/**
 * additive-expression:
 *     multiplicative-expression
 *     additive-expression + multiplicative-expression
 *     additive-expression - multiplicative-expression
 */
class AdditiveExpression {
public:
    enum AEOp {
        PLUS,
        MINUS
    };
    MultiplicativeExpression *multiExpr;
    std::vector<std::pair<AEOp, MultiplicativeExpression *>> exprs;
};

/**
 * shift-expression:
 *     additive-expression
 *     shift-expression << additive-expression
 *     shift-expression >> additive-expression
 */
class ShiftExpression {
public:
    enum SEOp {
        LESSLESS,
        GREATERGREATER
    };
    AdditiveExpression *addiExpr;
    std::vector<std::pair<SEOp, AdditiveExpression *>> exprs;
};

/**
 * relational-expression:
 *     shift-expression
 *     relational-expression < shift-expression
 *     relational-expression > shift-expression
 *     relational-expression <= shift-expression
 *     relational-expression >= shift-expression
 */
class RelationalExpression {
public:
    enum REOp {
        LESS,
        GREATER,
        LESSEQUAL,
        GREATEREQUAL
    };
    ShiftExpression *shiftExpr;
    std::vector<std::pair<REOp, ShiftExpression *>> exprs;
};

/**
 * equality-expression:
 *     relational-expression
 *     equality-expression == relational-expression
 *     equality-expression != relational-expression
 */
class EqualityExpression {
public:
    enum EEOp {
        EQUALEQUAL,
        EXCLAIMEQUAL
    };
    RelationalExpression *relatExpr;
    std::vector<std::pair<EEOp, RelationalExpression *>> exprs;
};

/**
 * and-expression:
 *     equality-expression
 *     and-expression & equality-expression
 */
class BitAndExpression {
public:
    std::vector<EqualityExpression *> exprs;
};

/**
 * exclusive-or-expression:
 *     and-expression
 *     exclusive-or-expression ^ and-expression
 */
class BitXorExpression {
public:
    std::vector<BitAndExpression *> exprs;
};

/**
 * inclusive-or-expression:
 *      exclusive-or-expression
 *      inclusive-or-expression | exclusive-or-expression
 */
class BitOrExpression {
public:
    std::vector<BitXorExpression *> exprs;
};

/**
 * logical-and-expression:
 *      inclusive-or-expression
 *      logical-and-expression && inclusive-or-expression
 */
class LogicalAndExpression {
public:
    std::vector<BitOrExpression *> exprs;
};

/**
 * logical-or-expression:
 *      logical-and-expression
 *      logical-or-expression || logical-and-expression
 */
class LogicalOrExpression {
public:
    std::vector<LogicalAndExpression *> exprs;
};

/**
 * conditional-expression:
 *      logical-or-expression
 *      logical-or-expression ? expression : conditional-expression
 */
class ConditionalExpression {
public:
    LogicalOrExpression *logiOrExpr;
    std::optional<Expression *> expr;
    std::optional<ConditionalExpression *> condExpr;
};

/**
 * assignment-operator:
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
enum AssignmentOperator {
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
 * assignment-expression:
 *     conditional-expression
 *     unary-expression assignment-operator assignment-expression
 */
class AssignmentExpression {
public:
    ConditionalExpression *condExpr;
    UnaryExpression *unaryExpr;
    AssignmentOperator op;
    AssignmentExpression *assignExpr;
};

/**
 * expression:
 *     assignment-expression
 *     expression , assignment-expression
 */
class Expression {
public:
    std::vector<AssignmentExpression *> exprs;
};

/**
 * constant-expression:
 *     conditional-expression
 */
class ConstantExpression {
public:
    ConditionalExpression *condExpr;
};

/**
 * declaration:
 *     declaration-specifiers init-declarator-list{opt} ;
 *
 * init-declarator-list:
 *     init-declarator
 *     init-declarator-list , init-declarator
 */
class Declaration {
public:
    DeclarationSpecifiers *declSpecs;
    std::vector<InitDeclarator *> initDecls;
};

/**
 * declaration-specifiers:
 *     storage-class-specifier declaration-specifiers{opt}
 *     type-specifier declaration-specifiers{opt}
 *     type-qualifier declaration-specifiers{opt}
 *     function-specifier declaration-specifiers{opt}
 */
class DeclarationSpecifier {
public:
    std::variant<StorageClassSpecifier *,
                 TypeSpecifier *,
                 TypeQualifier *,
                 FunctionSpecifier *>
        var;
};

class DeclarationSpecifiers {
public:
    std::vector<DeclarationSpecifier *> declSpecs;
};

/**
 * init-declarator:
 *     declarator
 *     declarator = initializer
 */
class InitDeclarator {
public:
    Declarator *declarator;
    std::optional<Initializer *> initializer;
};

/**
 * storage-class-specifier:
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
    Specifier spec;
};

/**
 * type-specifier:
 *     VOID
 *     CHAR
 *     SHORT
 *     INT
 *     LONG
 *     FLOAT
 *     DOUBLE
 *     SIGNED
 *     UNSIGNED
 *     struct-or-union-specifier
 *     enum-specifier
 *     typedef-name
 *
 * typedef-name:
 *     IDENTIFIER
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
    std::variant<PrimTypeKind,
                 StructOrUnionSpecifier *,
                 EnumSpecifier *,
                 std::string>
        var;
};

/**
 * struct-declarator:
 *     declarator
 *     declarator{opt} : constant-expression
 */
class StructDeclarator {
public:
    std::optional<Declarator *> declarator;
    std::optional<ConstantExpression *> constExpr;
};

/**
 * specifier-qualifier-list:
 *     type-specifier specifier-qualifier-list{opt}
 *     type-qualifier specifier-qualifier-list{opt}
 */
class SpecifierQualifier {
public:
    std::variant<TypeSpecifier *, TypeQualifier *> var;
};

/**
 * struct-declaration:
 *     specifier-qualifier-list struct-declarator-list ;
 *
 * struct-declarator-list:
 *     struct-declarator
 *     struct-declarator-list , struct-declarator
 */
class StructDeclaration {
public:
    std::vector<SpecifierQualifier *> specQualList;
    std::vector<StructDeclarator *> structDeclarators;
};

/**
 * struct-or-union-specifier:
 *     struct-or-union IDENTIFIER{opt} { struct-declaration-list }
 *     struct-or-union IDENTIFIER
 *
 * struct-or-union:
 *     STRUCT
 *     UNION
 *
 * struct-declaration-list:
 *     struct-declaration
 *     struct-declaration-list struct-declaration
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
 *     IDENTIFIER = constant-expression
 */
class Enumerator {
public:
    std::string ident;
    std::optional<ConstantExpression *> constExpr;
};

/**
 * enum-specifier:
 *     ENUM IDENTIFIER{opt} { enumerator-list }
 *     ENUM IDENTIFIER{opt} { enumerator-list , }
 *     ENUM IDENTIFIER
 *
 * enumerator-list:
 *     enumerator
 *     enumerator-list , enumerator
 */
class EnumSpecifier {
public:
    std::string ident;
    std::vector<Enumerator *> enumerators;
};

/**
 * type-qualifier:
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
    TypeQual typeQual;
};

/**
 * function-specifier:
 *     INLINE
 */
class FunctionSpecifier {
public:
    enum FuncSpec {
        INLINE
    };
    FuncSpec funcSpec;
};

/**
 * declarator:
 *     pointer{opt} direct-declarator
 */
class Declarator {
public:
    std::vector<Pointer *> pointers;
    DirectDeclarator *directDeclarator;
};

/**
 * direct-declarator:
 *     IDENTIFIER
 */
class DirectDeclaratorIdentifier {
public:
    std::string ident;
};

/**
 * direct-declarator:
 *     ( declarator )
 */
class DirectDeclaratorParentheses {
public:
    Declarator *declarator;
};

/**
 * direct-declarator:
 *     direct-declarator [ type-qualifier-list{opt} assignment-expression{opt} ]
 */
class DirectDeclaratorNoStaticOrAsterisk {
public:
    DirectDeclarator *directDeclarator;
    std::vector<TypeQualifier *> typeQuals;
    std::optional<AssignmentExpression *> assignExpr;
};

/**
 * direct-declarator:
 *     direct-declarator [ STATIC type-qualifier-list{opt} assignment-expression ]
 *     direct-declarator [ type-qualifier-list STATIC assignment-expression ]
 */
class DirectDeclaratorStatic {
public:
    DirectDeclarator *directDeclarator;
    bool staticFirst;
    std::vector<TypeQualifier *> typeQuals;
    AssignmentExpression *assignExpr;
};

/**
 * direct-declarator:
 *     direct-declarator [ type-qualifier-list{opt} * ]
 */
class DirectDeclaratorAsterisk {
public:
    DirectDeclarator *directDeclarator;
    std::vector<TypeQualifier *> typeQuals;
};

/**
 * direct-declarator:
 *     direct-declarator ( parameter-type-list )
 */
class DirectDeclaratorParameterTypeList {
public:
    DirectDeclarator *directDeclarator;
    ParameterTypeList *paramTypeList;
};

/**
 * direct-declarator:
 *     IDENTIFIER
 *     ( declarator )
 *     direct-declarator [ type-qualifier-list{opt} assignment-expression{opt} ]
 *     direct-declarator [ STATIC type-qualifier-list{opt} assignment-expression ]
 *     direct-declarator [ type-qualifier-list STATIC assignment-expression ]
 *     direct-declarator [ type-qualifier-list{opt} * ]
 *     direct-declarator ( parameter-type-list )
 *     direct-declarator ( identifier-list{opt} )
 *
 * "direct-declarator ( identifier-list )" here is used to support old-style declarator,
 * which we don't support. e.g.
 *     int old(a, b) int a; int b; { return a + b; }
 */
class DirectDeclarator {
public:
    std::variant<DirectDeclaratorIdentifier *,
                 DirectDeclaratorParentheses *,
                 DirectDeclaratorNoStaticOrAsterisk *,
                 DirectDeclaratorStatic *,
                 DirectDeclaratorAsterisk *,
                 DirectDeclaratorParameterTypeList *>
        var;
};

/**
 * pointer:
 *     * type-qualifier-list{opt}
 *     * type-qualifier-list{opt} pointer
 */
class Pointer {
public:
    std::vector<TypeQualifier *> typeQuals;
};

/**
 * parameter-type-list:
 *     parameter-list
 *     parameter-list , ...
 *
 * parameter-list:
 *     parameter-declaration
 *     parameter-list , parameter-declaration
 */
class ParameterTypeList {
public:
    std::vector<ParameterDeclaration *> paramList;
    bool hasEllipsis;
};

/**
 * parameter-declaration:
 *     declaration-specifiers declarator
 *     declaration-specifiers abstract-declarator{opt}
 */
class ParameterDeclaration {
public:
    DeclarationSpecifiers *declSpecs;
    std::variant<Declarator *, std::optional<AbstractDeclarator *>> var;
};

/**
 * type-name:
 *     specifier-qualifier-list abstract-declarator{opt}
 */
class TypeName {
public:
    std::vector<SpecifierQualifier *> specQualList;
    std::optional<AbstractDeclarator *> absDeclarator;
};

/**
 * abstract-declarator:
 *     pointer
 *     pointer{opt} direct-abstract-declarator
 */
class AbstractDeclarator {
public:
    std::vector<Pointer *> pointers;
    std::optional<DirectAbstractDeclarator *> directAbsDeclarator;
};

/**
 * direct-abstract-declarator:
 *     ( abstract-declarator )
 */
class DirectAbstractDeclaratorParentheses {
public:
    AbstractDeclarator *absDeclarator;
};

/**
 * direct-abstract-declarator:
 *     direct-abstract-declarator{opt} [ type-qualifier-list{opt} assignment-expression{opt} ]
 *     direct-abstract-declarator{opt} [ STATIC type-qualifier-list{opt} assignment-expression ]
 *     direct-abstract-declarator{opt} [ type-qualifier-list STATIC assignment-expression ]
 */
class DirectAbstractDeclaratorAssignExpr {
public:
    std::optional<DirectAbstractDeclarator *> directAbsDeclarator;
    std::vector<TypeQualifier *> typeQualList;
    std::optional<AssignmentExpression *> assignExpr;
    bool hasStatic;
    bool staticFirst;
};

/**
 * direct-abstract-declarator:
 *     direct-abstract-declarator{opt} [ * ]
 */
class DirectAbstractDeclaratorAsterisk {
public:
    std::optional<DirectAbstractDeclarator *> directAbsDeclarator;
};

/**
 * direct-abstract-declarator:
 *     direct-abstract-declarator{opt} ( parameter-type-list{opt} )
 */
class DirectAbstractDeclaratorParameterTypeList {
public:
    std::optional<DirectAbstractDeclarator *> directAbsDeclarator;
    std::optional<ParameterTypeList *> paramTypeList;
};

/**
 * direct-abstract-declarator:
 *     ( abstract-declarator )
 *     direct-abstract-declarator{opt} [ type-qualifier-list{opt} assignment-expression{opt} ]
 *     direct-abstract-declarator{opt} [ STATIC type-qualifier-list{opt} assignment-expression ]
 *     direct-abstract-declarator{opt} [ type-qualifier-list STATIC assignment-expression ]
 *     direct-abstract-declarator{opt} [ * ]
 *     direct-abstract-declarator{opt} ( parameter-type-list{opt} )
 */
class DirectAbstractDeclarator {
public:
    std::variant<DirectAbstractDeclaratorParentheses *,
                 DirectAbstractDeclaratorAssignExpr *,
                 DirectAbstractDeclaratorAsterisk *,
                 DirectAbstractDeclaratorParameterTypeList *>
        var;
};

/**
 * initializer:
 *     assignment-expression
 *     { initializer-list }
 *     { initializer-list , }
 */
class Initializer {
public:
    std::variant<AssignmentExpression *, InitializerList *> var;
};

/**
 * designator:
 *     [ constant-expression ]
 *     . IDENTIFIER
 */
class Designator {
public:
    std::variant<ConstantExpression *, std::string> var;
};

/**
 * initializer-list:
 *     designation{opt} initializer
 *     initializer-list , designation{opt} initializer
 */
class InitializerPair {
public:
    std::optional<std::vector<Designator *>> designation;
    Initializer *initializer;
};

/**
 * initializer-list:
 *     designation{opt} initializer
 *     initializer-list , designation{opt} initializer
 *
 * designation:
 *     designator-list =
 *
 * designator-list:
 *     designator
 *     designator-list designator
 *
 * designator:
 *     [ constant-expression ]
 *     . IDENTIFIER
 *
 * e.g.
 *     struct { int a[3], b; } w[] = { [0].a = {1}, [1].a[0] = 2 };
 * [0].a meaning designator designator
 */
class InitializerList {
public:
    std::vector<InitializerPair *> initializerPairs;
};

/**
 * labeled-statement:
 *     IDENTIFIER : statement
 *     CASE constant-expression : statement
 *     DEFAULT : statement
 */
class LabelStatement {
public:
    std::string ident;
    Statement *stmt;
};

class CaseStatement {
public:
    ConstantExpression *constExpr;
    Statement *stmt;
};

class DefaultStatement {
public:
    Statement *stmt;
};

/**
 * block-item:
 *     declaration
 *     statement
 */
class BlockItem {
public:
    std::variant<Statement *, Declaration *> var;
};

/**
 * compound-statement:
 *     { block-item-list{opt} }
 *
 * block-item-list:
 *     block-item
 *     block-item-list block-item
 */
class CompoundStatement {
public:
    std::vector<BlockItem *> blockItemList;
};

/**
 * expression-statement:
 *     expression{opt} ;
 */
class ExpressionStatement {
public:
    std::optional<Expression *> expr;
};

/**
 * selection-statement:
 *     IF ( expression ) statement
 *     IF ( expression ) statement ELSE statement
 *     SWITCH ( expression ) statement
 */
class IfStatement {
public:
    Expression *expr;
    Statement *thenStmt;
    std::optional<Statement *> elseStmt;
};

class SwitchStatement {
public:
    Expression *expr;
    Statement *stmt;
};

/**
 * iteration-statement:
 *     WHILE ( expression ) statement
 *     DO statement WHILE ( expression ) ;
 *     FOR ( expression-statement expression-statement expression{opt} ) statement
 *     FOR ( declaration expression-statement expression{opt} ) statement
 */
class WhileStatement {
public:
    Expression *expr;
    Statement *stmt;
};

class DoWhileStatement {
public:
    Statement *stmt;
    Expression *expr;
};

class ForStatement {
public:
    std::variant<ExpressionStatement *, Declaration *> init;
    ExpressionStatement *exprStmt;
    std::optional<Expression *> expr;
    Statement *stmt;
};

/**
 * jump-statement:
 *     GOTO IDENTIFIER ;
 *     CONTINUE ;
 *     BREAK ;
 *     RETURN expression{opt} ;
 */
class GotoStatement {
public:
    std::string ident;
};

class ContinueStatement {
public:
};

class BreakStatement {
public:
};

class ReturnStatement {
public:
    std::optional<Expression *> expr;
};

/**
 * statement:
 *     labeled-statement
 *     compound-statement
 *     expression-statement
 *     selection-statement
 *     iteration-statement
 *     jump-statement
 */
class Statement {
public:
    std::variant<LabelStatement *,
                 CaseStatement *,
                 DefaultStatement *,
                 CompoundStatement *,
                 ExpressionStatement *,
                 IfStatement *,
                 SwitchStatement *,
                 WhileStatement *,
                 DoWhileStatement *,
                 ForStatement *,
                 GotoStatement *,
                 ContinueStatement *,
                 BreakStatement *,
                 ReturnStatement *>
        var;
};

/**
 * function-definition:
 *     declaration-specifiers declarator declaration-list{opt} compound-statement
 *
 * "declaration-list" here is used to support old-style function definitions, which we don't support.
 * e.g.
 *     int old(a, b) int a; int b; { return a + b; }
 */
class FunctionDefinition {
public:
    DeclarationSpecifiers *declSpecs;
    Declarator *declarator;
    CompoundStatement *compoundStmt;
};

/**
 * external-declaration:
 *     function-definition
 *     declaration
 */
class ExternalDeclaration {
public:
    std::variant<Declaration *, FunctionDefinition *> var;
};

/**
 * translation-unit:
 *     external-declaration
 *     translation-unit external-declaration
 *
 * The entry of the syntax tree.
 */
class TranslationUnit {
public:
    std::vector<ExternalDeclaration *> externDecls;
};

#endif