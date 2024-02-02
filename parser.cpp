#include "parser.h"
#include "diagnostic.h"
#include <cassert>

void Parser::initBitSet() {
    setBitSet(firstOfSpecifierQualifier,
              // TypeQualifier
              TokenKind::TK_CONST,
              TokenKind::TK_RESTRICT,
              TokenKind::TK_VOLATILE,
              // TypeSpecifier
              TokenKind::TK_VOID,
              TokenKind::TK_CHAR,
              TokenKind::TK_SHORT,
              TokenKind::TK_INT,
              TokenKind::TK_LONG,
              TokenKind::TK_FLOAT,
              TokenKind::TK_DOUBLE,
              TokenKind::TK_SIGNED,
              TokenKind::TK_UNSIGNED,
              TokenKind::TK_STRUCT,
              TokenKind::TK_UNION,
              TokenKind::TK_ENUM,
              TokenKind::TK_IDENTIFIER);

    setBitSet(firstOfDeclarator,
              // Pointer
              TokenKind::TK_STAR,
              // DirectDeclarator
              TokenKind::TK_IDENTIFIER,
              TokenKind::TK_LPAR);
}

bool Parser::isInBitSet(TokenBitSet &bitset, TokenSeqConstIter &iter) {
    return bitset[(*iter)->getKind()];
}

TranslationUnit *Parser::parseTranslationUnit() {
    auto begin = cursor;
    TranslationUnit *transUnit = new TranslationUnit;
    while ((*cursor)->getKind() != TokenKind::TK_EOF) {
        // ; null statement
        if ((*cursor)->getKind() == TokenKind::TK_SEMI) {
            ++cursor;
            continue;
        }
        auto externDecl = parseExternalDeclaration();
        if (externDecl) {
            transUnit->extDecls.push_back(externDecl);
        }
    }
    return transUnit;
}

ExternalDeclaration *Parser::parseExternalDeclaration() {
    auto begin = cursor;
    auto declSpecs = parseDeclarationSpecifiers();
    if ((*cursor)->getKind() == TokenKind::TK_SEMI) {
        ++cursor;
        auto decl = new Declaration(declSpecs);
        return new ExternalDeclaration(decl);
    }
    auto declarator = parseDeclarator();
    // TODO
    return nullptr;
}

DeclSpecifiers *Parser::parseDeclarationSpecifiers() {
    DeclSpecifiers *declSpecs = new DeclSpecifiers;
    while (true) {
        switch ((*cursor)->getKind()) {
        case TokenKind::TK_TYPEDEF:
            declSpecs->addStorageClassSpecifier(new StorageClassSpecifier(StorageClassSpecifier::TYPEDEF));
            ++cursor;
            break;
        case TokenKind::TK_EXTERN:
            declSpecs->addStorageClassSpecifier(new StorageClassSpecifier(StorageClassSpecifier::EXTERN));
            ++cursor;
            break;
        case TokenKind::TK_STATIC:
            declSpecs->addStorageClassSpecifier(new StorageClassSpecifier(StorageClassSpecifier::STATIC));
            ++cursor;
            break;
        case TokenKind::TK_AUTO:
            declSpecs->addStorageClassSpecifier(new StorageClassSpecifier(StorageClassSpecifier::AUTO));
            ++cursor;
            break;
        case TokenKind::TK_REGISTER:
            declSpecs->addStorageClassSpecifier(new StorageClassSpecifier(StorageClassSpecifier::REGISTER));
            ++cursor;
            break;
        case TokenKind::TK_CONST:
            declSpecs->addTypeQualifier(new TypeQualifier(TypeQualifier::CONST));
            ++cursor;
            break;
        case TokenKind::TK_RESTRICT:
            declSpecs->addTypeQualifier(new TypeQualifier(TypeQualifier::RESTRICT));
            ++cursor;
            break;
        case TokenKind::TK_VOLATILE:
            declSpecs->addTypeQualifier(new TypeQualifier(TypeQualifier::VOLATILE));
            ++cursor;
            break;
        case TokenKind::TK_INLINE:
            declSpecs->addFuncSpecifier(new FuncSpecifier(FuncSpecifier::INLINE));
            ++cursor;
            break;
        case TokenKind::TK_VOID:
            declSpecs->addTypeSpecifier(new TypeSpecifier(TypeSpecifier::VOID));
            ++cursor;
            break;
        case TokenKind::TK_CHAR:
            declSpecs->addTypeSpecifier(new TypeSpecifier(TypeSpecifier::CHAR));
            ++cursor;
            break;
        case TokenKind::TK_SHORT:
            declSpecs->addTypeSpecifier(new TypeSpecifier(TypeSpecifier::SHORT));
            ++cursor;
            break;
        case TokenKind::TK_INT:
            declSpecs->addTypeSpecifier(new TypeSpecifier(TypeSpecifier::INT));
            ++cursor;
            break;
        case TokenKind::TK_LONG:
            declSpecs->addTypeSpecifier(new TypeSpecifier(TypeSpecifier::LONG));
            ++cursor;
            break;
        case TokenKind::TK_FLOAT:
            declSpecs->addTypeSpecifier(new TypeSpecifier(TypeSpecifier::FLOAT));
            ++cursor;
            break;
        case TokenKind::TK_DOUBLE:
            declSpecs->addTypeSpecifier(new TypeSpecifier(TypeSpecifier::DOUBLE));
            ++cursor;
            break;
        case TokenKind::TK_SIGNED:
            declSpecs->addTypeSpecifier(new TypeSpecifier(TypeSpecifier::SIGNED));
            ++cursor;
            break;
        case TokenKind::TK_UNSIGNED:
            declSpecs->addTypeSpecifier(new TypeSpecifier(TypeSpecifier::UNSIGNED));
            ++cursor;
            break;
        case TokenKind::TK_STRUCT:
        case TokenKind::TK_UNION: {
            auto structUnionSpec = parseStructOrUnionSpecifier();
            if (structUnionSpec) {
                declSpecs->addTypeSpecifier(new TypeSpecifier(structUnionSpec));
            }
        } break;
        case TokenKind::TK_ENUM: {
            auto enumSpec = parseEnumSpecifier();
            if (enumSpec) {
                declSpecs->addTypeSpecifier(new TypeSpecifier(enumSpec));
            }
        } break;
        case TokenKind::TK_IDENTIFIER:
            declSpecs->addTypeSpecifier(new TypeSpecifier((*cursor)->getStr()));
            ++cursor;
            break;
        default:
            return declSpecs;
        }
    }
}

Declarator *Parser::parseDeclarator() {
}

StructOrUnionSpecifier *Parser::parseStructOrUnionSpecifier() {
    auto begin = cursor;
    StructOrUnionSpecifier *structUnionSpec = new StructOrUnionSpecifier;
    structUnionSpec->isUnion = (*cursor)->getKind() == TokenKind::TK_UNION;
    ++cursor; // Consume STRUCT or UNION.
    switch ((*cursor)->getKind()) {
    case TokenKind::TK_IDENTIFIER:
        structUnionSpec->ident = (*cursor)->getStr();
        if ((*cursor)->getKind() != TokenKind::TK_LBRACE) {
            return structUnionSpec;
        }
    case TokenKind::TK_LBRACE:
        ++cursor; // Consume the '{'.
        while (isInBitSet(firstOfSpecifierQualifier, cursor) || (*cursor)->getKind() == TokenKind::TK_SEMI) {
            // struct_declaration or just ';'
            auto structDecl = parseStructDeclaration();
            if (structDecl) {
                structUnionSpec->structDeclList.push_back(structDecl);
            }
        }
        assert((*cursor)->getKind() == TokenKind::TK_RBRACE);
        ++cursor; // Consume the '}'.
        return structUnionSpec;
    default:
        error((*cursor)->getLoc(), "parse struct_or_union_specifier error");
        return nullptr;
    }
}

EnumSpecifier *Parser::parseEnumSpecifier() {
}

StructDeclaration *Parser::parseStructDeclaration() {
    auto begin = cursor;

    // To support struct like "struct id { ; };" which contains empty declaration.
    if ((*cursor)->getKind() == TokenKind::TK_SEMI) {
        ++cursor; // Consume the ';'.
        return nullptr;
    }

    StructDeclaration *structDecl = new StructDeclaration;
    // Parse specifier_qualifier_list
    while (isInBitSet(firstOfSpecifierQualifier, cursor)) {
        auto specQual = parseSpecifierQualifier();
        if (specQual) {
            structDecl->specQualList.push_back(specQual);
        }
    }
    // Parse struct_declarator_list
    while (isInBitSet(firstOfDeclarator, cursor) || (*cursor)->getKind() == TokenKind::TK_COLON) {
        auto structDeclarator = parseStructDeclarator();
        if (structDeclarator) {
            structDecl->structDeclarators.push_back(structDeclarator);
        }
        if ((*cursor)->getKind() == TokenKind::TK_COMMA) {
            ++cursor; // Consume the ','.
        } else {
            break;
        }
    }
    assert((*cursor)->getKind() == TokenKind::TK_SEMI);
    ++cursor; // Consume the ';'.
    return structDecl;
}

SpecifierQualifier *Parser::parseSpecifierQualifier() {
    auto begin = cursor;
    SpecifierQualifier *specQual = new SpecifierQualifier;
    switch ((*cursor)->getKind()) {
    case TokenKind::TK_CONST:
        specQual->var = new TypeQualifier(TypeQualifier::CONST);
        ++cursor;
        break;
    case TokenKind::TK_RESTRICT:
        specQual->var = new TypeQualifier(TypeQualifier::RESTRICT);
        ++cursor;
        break;
    case TokenKind::TK_VOLATILE:
        specQual->var = new TypeQualifier(TypeQualifier::VOLATILE);
        ++cursor;
        break;
    case TokenKind::TK_VOID:
        specQual->var = new TypeSpecifier(TypeSpecifier::VOID);
        ++cursor;
        break;
    case TokenKind::TK_CHAR:
        specQual->var = new TypeSpecifier(TypeSpecifier::CHAR);
        ++cursor;
        break;
    case TokenKind::TK_SHORT:
        specQual->var = new TypeSpecifier(TypeSpecifier::SHORT);
        ++cursor;
        break;
    case TokenKind::TK_INT:
        specQual->var = new TypeSpecifier(TypeSpecifier::INT);
        ++cursor;
        break;
    case TokenKind::TK_LONG:
        specQual->var = new TypeSpecifier(TypeSpecifier::LONG);
        ++cursor;
        break;
    case TokenKind::TK_FLOAT:
        specQual->var = new TypeSpecifier(TypeSpecifier::FLOAT);
        ++cursor;
        break;
    case TokenKind::TK_DOUBLE:
        specQual->var = new TypeSpecifier(TypeSpecifier::DOUBLE);
        ++cursor;
        break;
    case TokenKind::TK_SIGNED:
        specQual->var = new TypeSpecifier(TypeSpecifier::SIGNED);
        ++cursor;
        break;
    case TokenKind::TK_UNSIGNED:
        specQual->var = new TypeSpecifier(TypeSpecifier::UNSIGNED);
        ++cursor;
        break;
    case TokenKind::TK_STRUCT:
    case TokenKind::TK_UNION: {
        auto structUnionSpec = parseStructOrUnionSpecifier();
        if (structUnionSpec) {
            specQual->var = new TypeSpecifier(structUnionSpec);
        }
    } break;
    case TokenKind::TK_ENUM: {
        auto enumSpec = parseEnumSpecifier();
        if (enumSpec) {
            specQual->var = new TypeSpecifier(enumSpec);
        }
    } break;
    case TokenKind::TK_IDENTIFIER:
        specQual->var = new TypeSpecifier((*cursor)->getStr());
        ++cursor;
        break;
    default:
        return nullptr;
    }
    return specQual;
}

StructDeclarator *Parser::parseStructDeclarator() {
    auto begin = cursor;
    StructDeclarator *structDeclarator = new StructDeclarator;
    if ((*cursor)->getKind() == TokenKind::TK_COLON) {
        ++cursor;
        auto condExpr = parseConditionalExpr();
        if (condExpr) {
            structDeclarator->constExpr = new ConstantExpr(condExpr);
        }
        return structDeclarator;
    } else if (isInBitSet(firstOfDeclarator, cursor)) {
        auto declarator = parseDeclarator();
        if (declarator) {
            structDeclarator->declarator = declarator;
        }
        if ((*cursor)->getKind() == TokenKind::TK_COLON) {
            ++cursor; // Consume the ':'.
        } else {
            
        }
    } else {
        return nullptr;
    }
}

ConditionalExpr *Parser::parseConditionalExpr() {
}