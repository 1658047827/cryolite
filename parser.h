#ifndef _CRYOLITE_PARSER_H_
#define _CRYOLITE_PARSER_H_

#include "ast.h"
#include "token.h"
#include <bitset>

using TokenBitSet = std::bitset<NUM_TOKENS>;

class Scope {
};

class DeclSpec {
public:
    // storage-class-specifier
    enum SCS {
        SCS_UNSPECIFIED,
        SCS_TYPEDEF,
        SCS_EXTERN,
        SCS_STATIC,
        SCS_AUTO,
        SCS_REGISTER
    };

    // type-specifier
    enum TSW {
        TSW_UNSPECIFIED,
        TSW_SHORT,
        TSW_LONG,
        TSW_LONGLONG
    };

    enum TSS {
        TSS_UNSPECIFIED,
        TSS_SIGNED,
        TSS_UNSIGNED
    };

    enum TST {
        TST_UNSPECIFIED,
        TST_VOID,
        TST_CHAR,
        TST_INT,
        TST_FLOAT,
        TST_DOUBLE,
        TST_BOOL,
        TST_ENUM,
        TST_UNION,
        TST_STRUCT,
        TST_TYPENAME, // Typedef, enum name, etc.
        TST_TYPEOF_TYPE,
        TST_TYPEOF_EXPR,
        TST_ERROR // erroneous type
    };

    // type-qualifiers
    enum TQ {
        TQ_UNSPECIFIED = 0,
        TQ_CONST = 1,
        TQ_RESTRICT = 2,
        TQ_VOLATILE = 4
    };

    /**
     * ParsedSpecifiers - Flags to query which specifiers were applied. This is
     * returned by getParsedSpecifiers.
     */
    enum ParsedSpecifiers {
        PQ_NONE = 0,
        PQ_STORAGE_CLASS_SPECIFIER = 1,
        PQ_TYPE_SPECIFIER = 2,
        PQ_TYPE_QUALIFIER = 4,
        PQ_FUNCTION_SPECIFIER = 8
    };

    // storage-class-specifier
    SCS storageClassSpec;

    // type-specifier
    TSW typeSpecWidth;
    TSS typeSpecSign;
    TST typeSpecType;
    bool typeSpecOwned;

    // type-qualifiers
    unsigned typeQualifiers; // Bitwise OR of TQ.

    // function-specifier
    bool fsInlineSpecified;

    SourceLocation storageClassSpecLoc;
    SourceLocation tswLoc, tscLoc, tssLoc, tstLoc;
    SourceLocation tqConstLoc, tqRestrictLoc, tqVolatileLoc;
    SourceLocation fsInlineLoc;

    DeclSpec()
        : storageClassSpec(SCS_UNSPECIFIED),
          typeSpecWidth(TSW_UNSPECIFIED),
          typeSpecSign(TSS_UNSPECIFIED),
          typeSpecType(TST_UNSPECIFIED),
          typeSpecOwned(false),
          typeQualifiers(TQ_UNSPECIFIED),
          fsInlineSpecified(false) {}

    bool hasTypeSpecifier() {
        return typeSpecType != TST_UNSPECIFIED ||
               typeSpecWidth != TSW_UNSPECIFIED ||
               typeSpecSign != TSS_UNSPECIFIED;
    }

    /**
     * getParsedSpecifiers - Return a bitmask of which flavors of specifiers this
     * DeclSpec includes.
     */
    unsigned getParsedSpecifiers();

    /**
     * isEmpty - Return true if this declaration specifier is completely empty:
     * no tokens were parsed in the production of it.
     */
    bool isEmpty() { return getParsedSpecifiers() == PQ_NONE; }

    /**
     * These methods set the specified attribute of the DeclSpec and
     * return false if there was no error.
     */
    bool setStorageClassSpec(SCS s, SourceLocation loc);
    bool setTypeSpecWidth(TSW w, SourceLocation loc);
    bool setTypeSpecSign(TSS s, SourceLocation loc);
    bool setTypeSpecType(TST t, SourceLocation loc, bool owned);
    bool setTypeSpecError();
    bool setTypeQual(TQ t, SourceLocation loc);
    bool setFunctionSpecInline(SourceLocation loc);

    /**
     * finish - This does final analysis of the declspec, issuing diagnostics.
     * After calling this method, DeclSpec is guaranteed self-consistent, even if an error occurred.
     */
    void finish();

    /**
     * isMissingDeclaratorOk - This checks if this DeclSpec can stand alone,
     * without a Declarator. Only tag declspecs can stand alone.
     */
    bool isMissingDeclaratorOk();
};

struct DeclGroup {
    std::vector<Decl *> decls;
};

class Parser {
public:
    Parser(TokenSequence &ts)
        : tksq(ts), cursor(ts.cBegin()) { initBitSet(); }

    /**
     * Expressions
     */
    Expr *parseExpression();
    Expr *parseAssignmentExpression();
    Expr *parseConditionalExpression();
    // parseSimpleBinaryExpression - Parse very simple binary expressions like
    // LogicalAndExpression, LogicalOrExpression, BitOrExpression etc.
    // They only involve one type of binary operator.
    Expr *parseSimpleBinaryExpression(Expr *(Parser::*parseTerm)(), TokenKind kind, BinaryOpKind op);
    Expr *parseLogicalOrExpression();
    Expr *parseLogicalAndExpression();
    Expr *parseBitOrExpression();
    Expr *parseBitXorExpression();
    Expr *parseBitAndExpression();
    Expr *parseEqualityExpression();
    Expr *parseRelationalExpression();
    Expr *parseShiftExpression();
    Expr *parseAdditiveExpression();
    Expr *parseMultiplicativeExpression();
    Expr *parseCastExpression();
    // Parse unary expressions.
    Expr *parseUnaryExpression();
    UnaryExpr *parsePrefixIncrementOrDecrement(UnaryOpKind op);
    UnaryExpr *parseUnaryOperatorExpression(UnaryOpKind op);
    SizeofExpr *parseSizeof();
    // Parse postfix expressions.
    Expr *parsePostfixExpression();
    Expr *parsePostfixExpressionSuffix(Expr *expr);
    Expr *parseCompoundLiteral();
    Expr *parseArraySubscripting();
    // Parse primary expressions.
    Expr *parsePrimaryExpression();
    DeclRefExpr *parseIdentifier();
    Expr *parseParenthesesExpression();
    IntegerConstant *parseIntegerConstant(NumericLiteralParser &parser);
    FloatingConstant *parseFloatingConstant(NumericLiteralParser &parser);
    CharacterConstant *parseCharacterConstant();
    StringLiteral *parseStringLiterals();

    /**
     * Declarations
     */
    void parseDeclarationSpecifiers(DeclSpec &ds);
    void parseSpecifierQualifierList(DeclSpec &ds);
    QualType parseTypeName();

    /**
     * Statements and blocks
     */
    Stmt *parseBreakStmt();
    Stmt *parseContinueStmt();

    /**
     * External definitions
     */
    TransUnit *parseTranslationUnit();

private:
    inline bool isKind(TokenSeqConstIter &iter, TokenKind kind);
    void expect(TokenSeqConstIter &iter, TokenKind kind);
    void initBitSet();
    template <typename... Args>
    void setBitSet(TokenBitSet &bitset, Args... values) {
        (bitset.set(values), ...);
    }
    bool isInBitSet(TokenBitSet &bitset, TokenSeqConstIter &iter);
    bool isFirstOfTypeName(TokenSeqConstIter &iter);

    TokenSequence &tksq;
    TokenSeqConstIter cursor;

    TokenBitSet firstOfSpecifierQualifier;
    TokenBitSet firstOfDeclarator;
};

#endif