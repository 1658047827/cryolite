#include "syntax.h"

void DeclSpecifiers::addStorageClassSpecifier(StorageClassSpecifier *storageClassSpec) {
    specs.push_back(new DeclSpecifier(storageClassSpec));
}

void DeclSpecifiers::addTypeSpecifier(TypeSpecifier *typeSpec) {
    specs.push_back(new DeclSpecifier(typeSpec));
}

void DeclSpecifiers::addTypeQualifier(TypeQualifier *typeQual) {
    specs.push_back(new DeclSpecifier(typeQual));
}

void DeclSpecifiers::addFuncSpecifier(FuncSpecifier *funcSpec) {
    specs.push_back(new DeclSpecifier(funcSpec));
}