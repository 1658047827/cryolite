#include "parser.h"

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
    DeclSpecifier *spec;
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
}

EnumSpecifier *Parser::ParseEnumSpecifier() {
}