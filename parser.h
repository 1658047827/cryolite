#ifndef _CRYOLITE_PARSER_H_
#define _CRYOLITE_PARSER_H_

#include "semantic.h"
#include <bitset>
#include <variant>

using TokenBitSet = std::bitset<NUM_TOKENS>;

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
     * ParsedSpecifiers - Flags to query which specifiers were applied.
     * This is returned by getParsedSpecifiers.
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
    unsigned char typeQualifiers; // Bitwise OR of TQ.

    // function-specifier
    bool fsInlineSpecified;

    // typeRep - For a typedef or struct, it might contain the declaration for these.
    void *typeRep;

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

    void clearStorageClassSpecs() {
        storageClassSpec = SCS_UNSPECIFIED;
        storageClassSpecLoc = SourceLocation();
    }

    void clearFunctionSpecs() {
        fsInlineSpecified = false;
        fsInlineLoc = SourceLocation();
    }

    bool hasTypeSpecifier() const {
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
    bool setTypeSpecType(TST t, SourceLocation loc, void *rep = nullptr, bool owned = false);
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

struct DeclaratorChunk {
    enum ChunkKind {
        POINTER,
        ARRAY,
        FUNCTION
    } chunkKind;
    SourceLocation loc;

    // enum Qualifier : unsigned char {
    //     CONST = 1 << 0,
    //     RESTRICT = 1 << 1,
    //     VOLATILE = 1 << 2,
    // };

    struct PointerTypeInfo {
        unsigned char typeQuals;
    };

    struct ArrayTypeInfo {
        unsigned char typeQuals;
        // hasStaic - True if this dimension included the 'static' keyword.
        bool hasStaic;
        // isStar - True if this dimension was [*].  In this case, NumElts is null.
        bool isStar;
        // numElts - The size of the array, or null if [] or [*] was specified.
        void *numElts;
    };

    struct ParamInfo {
        IdentifierInfo *ident;
        SourceLocation identLoc;
    };

    struct FunctionTypeInfo {
        unsigned char typeQuals;
        // hasPrototype - This is true if the function had at least one typed argument.
        bool hasPrototype;
        // isVariadic - If this function has a prototype, and if that proto ends with ',...)'.
        bool isVariadic;
        std::vector<ParamInfo> argInfo;
    };

    union {
        PointerTypeInfo ptr;
        ArrayTypeInfo arr;
        FunctionTypeInfo fun;
    };

    DeclaratorChunk() {}
    DeclaratorChunk(const DeclaratorChunk &dc)
        : chunkKind(dc.chunkKind), loc(dc.loc) {
        switch (dc.chunkKind) {
        case POINTER:
            ptr = dc.ptr;
            break;
        case ARRAY:
            arr = dc.arr;
            break;
        case FUNCTION:
            fun = dc.fun;
            break;
        }
    }
    ~DeclaratorChunk() {}

    static DeclaratorChunk getPointer(unsigned char typeQuals, SourceLocation loc);

    static DeclaratorChunk getArray(unsigned char typeQuals,
                                    bool isStatic,
                                    bool isStar,
                                    void *numElts,
                                    SourceLocation loc);

    static DeclaratorChunk getFunction(unsigned char typeQuals,
                                       bool hasProto,
                                       bool isVariadic,
                                       SourceLocation loc);
};

class Declarator {
public:
    enum TheContext {
        FILE_CONTEXT,      // File scope declaration.
        PROTOTYPE_CONTEXT, // Within a function prototype.
        TYPE_NAME_CONTEXT, // Abstract declarator for types.
        MEMBER_CONTEXT,    // Struct/Union field.
        BLOCK_CONTEXT,     // Declaration within a block in a function.
        FOR_CONTEXT,       // Declaration within first part of a for loop.
    };

    enum DeclaratorKind {
        DK_ABSTRACT, // An abstract declarator (has no identifier)
        DK_NORMAL    // A normal declarator (has an identifier).
    };

    // ds - The declaration specifiers that this declarator was declared with.
    const DeclSpec &ds;
    std::string identifier;
    SourceLocation identifierLoc;
    // context - Where we are parsing this declarator.
    TheContext context;
    // kind - What kind of declarator this is.
    DeclaratorKind kind;
    std::vector<DeclaratorChunk> declTypeInfo;
    bool invalidType;
    // groupingParens - Set by Parser::ParseParenDeclarator().
    bool groupingParens;

    Declarator(const DeclSpec &ds, TheContext c)
        : ds(ds), context(c), kind(DK_ABSTRACT),
          invalidType(ds.typeSpecType == DeclSpec::TST_ERROR),
          groupingParens(false) {}

    /**
     * mayOmitIdentifier - Return true if the identifier is either optional or
     * not allowed. This is true for typenames, prototypes.
     */
    bool mayOmitIdentifier() {
        return context == TYPE_NAME_CONTEXT || context == PROTOTYPE_CONTEXT;
    }

    /**
     * mayHaveIdentifier - Return true if the identifier is either optional or
     * required. This is true for normal declarators and prototypes, but not
     * typenames.
     */
    bool mayHaveIdentifier() {
        return context != TYPE_NAME_CONTEXT;
    }

    /**
     * isPastIdentifier - Return true if we have parsed beyond the point where
     * the name would appear. (This may happen even if we haven't actually parsed
     * a name, perhaps because this context doesn't require one.)
     */
    bool isPastIdentifier() { return identifierLoc.isValid(); }

    /**
     * hasName - Whether this declarator has a name, which is an identifier.
     */
    bool hasName() { return kind != DK_ABSTRACT; }

    void setIdentifier(const std::string &id, SourceLocation loc) {
        identifier = id;
        identifierLoc = loc;
        if (id.size() > 0)
            kind = DK_NORMAL;
        else
            kind = DK_ABSTRACT;
    }

    bool isInvalidType() { return invalidType || ds.typeSpecType == DeclSpec::TST_ERROR; }
};

struct FieldDeclarator {
    Declarator d;
    Expr *bitFieldWidth;
    explicit FieldDeclarator(DeclSpec &ds)
        : d(ds, Declarator::MEMBER_CONTEXT), bitFieldWidth(nullptr) {}
};

QualType convertDeclSpecToType(const DeclSpec &ds, SourceLocation loc, bool &isInvalid);
QualType getTypeForDeclarator(Declarator &d);

struct DeclGroup {
public:
    std::vector<Decl *> decls;
};

class Parser {
public:
    Parser(TokenSequence &ts, Semantic &sema)
        : tksq(ts), sema(sema) {
        cursor = ts.cBegin();
        tok = **cursor;
        numCachedScopes = 0;
        curScope = nullptr;
        initBitSet();
    }

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
    enum TagUseKind {
        DEFINITION,
        DECLARATION,
        REFERENCE
    };
    void parseRecordSpecifier(SourceLocation loc, DeclSpec &ds);

    void parseDeclarationSpecifiers(DeclSpec &ds);

    DeclGroup *parseDeclaration(Declarator::TheContext context);
    void parseDeclarator(Declarator &d);
    void parseParenDeclarator(Declarator &d);
    void parseBracketDeclarator(Declarator &d);
    void parseDeclaratorInternal(Declarator &d, void (Parser::*directDeclParser)(Declarator &));
    void parseDirectDeclarator(Declarator &d);
    DeclGroup *parseInitDeclaratorListAfterFirstDeclarator(Declarator &d);
    void parseEnumSpecifier(SourceLocation loc, DeclSpec &ds);

    RecordDecl *parseStructUnionBody(SourceLocation loc, DeclSpec::TST specType);
    void parseStructDeclaration(DeclSpec &ds, std::vector<FieldDeclarator> &fields);

    void parseSpecifierQualifierList(DeclSpec &ds);
    void parseTypeQualifierListOpt(DeclSpec &ds);
    QualType parseTypeName();

    /**
     * Statements and blocks
     */

    /**
     * External definitions
     */
    TransUnit *parseTranslationUnit();

private:
    void expect(TokenSeqConstIter &iter, TokenKind kind);
    void initBitSet();
    template <typename... Args>
    void setBitSet(TokenBitSet &bitset, Args... values) {
        (bitset.set(values), ...);
    }
    bool isInBitSet(TokenBitSet &bitset, TokenSeqConstIter &iter);
    bool isFirstOfTypeName(TokenSeqConstIter &iter);

    // consumeToken - Move cursor to the next one and return the location of the consumed token.
    SourceLocation consumeToken() {
        prevTokenLoc = tok.getLoc();
        ++cursor;
        tok = **cursor;
        return prevTokenLoc;
    }

    // skipUntil - Read tokens until we get to the specified token, then consume
    // it (unless dontConsume is true). Because we cannot guarantee that the
    // token will ever occur, this skips to the next token, or to some likely
    // good stopping point. If stopAtSemi is true, skipping will stop at a ';'.
    //
    // If skipUntil finds the specified token, it returns true, otherwise it
    // returns false.
    bool skipUntil(TokenKind t, bool stopAtSemi = true, bool dontConsume = false) {
        return skipUntil(&t, 1, stopAtSemi, dontConsume);
    }
    bool skipUntil(const TokenKind *toks, unsigned numToks, bool stopAtSemi = true, bool dontConsume = false);

    void enterScope(unsigned char scopeFlags);
    void exitScope();

    // tok - The current token we are peeking ahead.
    // All parsing methods assume that this is valid.
    Token tok;

    // prevTokenLoc - The location of the token we previously consumed.
    SourceLocation prevTokenLoc;

    Semantic &sema;

    Scope *curScope;
    // scopeCache - Cache scopes to reduce malloc traffic.
    static const unsigned scopeCacheSize = 16;
    Scope *scopeCache[scopeCacheSize];
    unsigned numCachedScopes;

    TokenSequence &tksq;
    TokenSeqConstIter cursor;

    TokenBitSet firstOfSpecifierQualifier;
    TokenBitSet firstOfDeclarator;
};

#endif