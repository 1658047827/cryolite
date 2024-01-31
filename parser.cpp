#include "parser.h"
#include "diagnostic.h"
#include <cassert>

void Parser::InitBitSet() {
    SetBitSet(firstOfSpecifierQualifier,
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
}

bool Parser::IsInBitSet(TokenBitSet &bitset, TokenSeqConstIter &iter) {
    return bitset[(*iter)->GetKind()];
}

TranslationUnit *Parser::ParseTranslationUnit() {
    auto begin = cursor;
    TranslationUnit *transUnit = new TranslationUnit;
    while ((*cursor)->GetKind() != TokenKind::TK_EOF) {
        // ; null statement
        if ((*cursor)->GetKind() == TokenKind::TK_SEMI) {
            ++cursor;
            continue;
        }
        auto externDecl = ParseExternalDeclaration();
        if (externDecl) {
            transUnit->extDecls.push_back(externDecl);
        }
    }
    return transUnit;
}

ExternalDeclaration *Parser::ParseExternalDeclaration() {
    auto begin = cursor;
    auto declSpecs = ParseDeclarationSpecifiers();
    if ((*cursor)->GetKind() == TokenKind::TK_SEMI) {
        ++cursor;
        auto decl = new Declaration(declSpecs);
        return new ExternalDeclaration(decl);
    }
    auto declarator = ParseDeclarator();

    // TODO
    return nullptr;
}

DeclSpecifiers *Parser::ParseDeclarationSpecifiers() {
    DeclSpecifiers *declSpecs = new DeclSpecifiers;
    while (true) {
        switch ((*cursor)->GetKind()) {
        case TokenKind::TK_TYPEDEF:
            declSpecs->AddStorageClassSpecifier(new StorageClassSpecifier(StorageClassSpecifier::TYPEDEF));
            ++cursor;
            break;
        case TokenKind::TK_EXTERN:
            declSpecs->AddStorageClassSpecifier(new StorageClassSpecifier(StorageClassSpecifier::EXTERN));
            ++cursor;
            break;
        case TokenKind::TK_STATIC:
            declSpecs->AddStorageClassSpecifier(new StorageClassSpecifier(StorageClassSpecifier::STATIC));
            ++cursor;
            break;
        case TokenKind::TK_AUTO:
            declSpecs->AddStorageClassSpecifier(new StorageClassSpecifier(StorageClassSpecifier::AUTO));
            ++cursor;
            break;
        case TokenKind::TK_REGISTER:
            declSpecs->AddStorageClassSpecifier(new StorageClassSpecifier(StorageClassSpecifier::REGISTER));
            ++cursor;
            break;
        case TokenKind::TK_CONST:
            declSpecs->AddTypeQualifier(new TypeQualifier(TypeQualifier::CONST));
            ++cursor;
            break;
        case TokenKind::TK_RESTRICT:
            declSpecs->AddTypeQualifier(new TypeQualifier(TypeQualifier::RESTRICT));
            ++cursor;
            break;
        case TokenKind::TK_VOLATILE:
            declSpecs->AddTypeQualifier(new TypeQualifier(TypeQualifier::VOLATILE));
            ++cursor;
            break;
        case TokenKind::TK_INLINE:
            declSpecs->AddFuncSpecifier(new FuncSpecifier(FuncSpecifier::INLINE));
            ++cursor;
            break;
        case TokenKind::TK_VOID:
            declSpecs->AddTypeSpecifier(new TypeSpecifier(TypeSpecifier::VOID));
            ++cursor;
            break;
        case TokenKind::TK_CHAR:
            declSpecs->AddTypeSpecifier(new TypeSpecifier(TypeSpecifier::CHAR));
            ++cursor;
            break;
        case TokenKind::TK_SHORT:
            declSpecs->AddTypeSpecifier(new TypeSpecifier(TypeSpecifier::SHORT));
            ++cursor;
            break;
        case TokenKind::TK_INT:
            declSpecs->AddTypeSpecifier(new TypeSpecifier(TypeSpecifier::INT));
            ++cursor;
            break;
        case TokenKind::TK_LONG:
            declSpecs->AddTypeSpecifier(new TypeSpecifier(TypeSpecifier::LONG));
            ++cursor;
            break;
        case TokenKind::TK_FLOAT:
            declSpecs->AddTypeSpecifier(new TypeSpecifier(TypeSpecifier::FLOAT));
            ++cursor;
            break;
        case TokenKind::TK_DOUBLE:
            declSpecs->AddTypeSpecifier(new TypeSpecifier(TypeSpecifier::DOUBLE));
            ++cursor;
            break;
        case TokenKind::TK_SIGNED:
            declSpecs->AddTypeSpecifier(new TypeSpecifier(TypeSpecifier::SIGNED));
            ++cursor;
            break;
        case TokenKind::TK_UNSIGNED:
            declSpecs->AddTypeSpecifier(new TypeSpecifier(TypeSpecifier::UNSIGNED));
            ++cursor;
            break;
        case TokenKind::TK_STRUCT:
        case TokenKind::TK_UNION: {
            auto structUnionSpec = ParseStructOrUnionSpecifier();
            if (structUnionSpec) {
                declSpecs->AddTypeSpecifier(new TypeSpecifier(structUnionSpec));
            }
        } break;
        case TokenKind::TK_ENUM: {
            auto enumSpec = ParseEnumSpecifier();
            if (enumSpec) {
                declSpecs->AddTypeSpecifier(new TypeSpecifier(enumSpec));
            }
        } break;
        case TokenKind::TK_IDENTIFIER:
            declSpecs->AddTypeSpecifier(new TypeSpecifier((*cursor)->GetStr()));
            ++cursor;
            break;
        default:
            return declSpecs;
        }
    }
}

Declarator *Parser::ParseDeclarator() {
}

StructOrUnionSpecifier *Parser::ParseStructOrUnionSpecifier() {
    auto begin = cursor;
    StructOrUnionSpecifier *structUnionSpec = new StructOrUnionSpecifier;
    structUnionSpec->isUnion = (*cursor)->GetKind() == TokenKind::TK_UNION;
    ++cursor; // Consume STRUCT or UNION.
    switch ((*cursor)->GetKind()) {
    case TokenKind::TK_IDENTIFIER:
        structUnionSpec->ident = (*cursor)->GetStr();
        if ((*cursor)->GetKind() != TokenKind::TK_LBRACE) {
            return structUnionSpec;
        }
    case TokenKind::TK_LBRACE:
        ++cursor; // Consume the '{'.
        while (IsInBitSet(firstOfSpecifierQualifier, cursor)) {
            auto structDecl = ParseStructDeclaration();
            if (structDecl) {
                structUnionSpec->structDeclList.push_back(structDecl);
            }
        }
        assert((*cursor)->GetKind() == TokenKind::TK_RBRACE);
        ++cursor; // Consume the '}'.
        return structUnionSpec;
    default:
        Error((*cursor)->GetLoc(), "parse struct_or_union_specifier error");
        return nullptr;
    }
}

EnumSpecifier *Parser::ParseEnumSpecifier() {
}

StructDeclaration *Parser::ParseStructDeclaration() {
}