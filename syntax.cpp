#include "syntax.h"

void DeclSpecifiers::AddStorageClassSpecifier(StorageClassSpecifier *storageClassSpec) {
    specs.push_back(new DeclSpecifier(storageClassSpec));
}

void DeclSpecifiers::AddTypeSpecifier(TypeSpecifier *typeSpec) {
    specs.push_back(new DeclSpecifier(typeSpec));
}

void DeclSpecifiers::AddTypeQualifier(TypeQualifier *typeQual) {
    specs.push_back(new DeclSpecifier(typeQual));
}

void DeclSpecifiers::AddFuncSpecifier(FuncSpecifier *funcSpec) {
    specs.push_back(new DeclSpecifier(funcSpec));
}