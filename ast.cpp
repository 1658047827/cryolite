#include "ast.h"

UnaryExpr::UnaryExpr(const SourceLocation &loc, UnaryOpKind op, Expr *expr)
    : Expr(loc, QualType()), op(op), operand(expr) {}

BinaryExpr::BinaryExpr(const SourceLocation &loc, BinaryOpKind op, Expr *lhs, Expr *rhs)
    : Expr(loc, QualType()), op(op), lhs(lhs), rhs(rhs) {}

TernaryExpr::TernaryExpr(const SourceLocation &loc, Expr *condExpr, Expr *trueExpr, Expr *falseExpr)
    : Expr(loc, QualType()), condExpr(condExpr), trueExpr(trueExpr), falseExpr(falseExpr) {}

IntegerConstant::IntegerConstant(const SourceLocation &loc, const QualType &qt, unsigned long long val)
    : Expr(loc, qt), value(val) {}

FloatingConstant::FloatingConstant(const SourceLocation &loc, const QualType &qt, long double val)
    : Expr(loc, qt), value(val) {}

CharacterConstant::CharacterConstant(const SourceLocation &loc, const QualType &qt, unsigned val)
    : Expr(loc, qt), value(val) {}

StringLiteral::StringLiteral(const SourceLocation &loc, const QualType &qt, const std::string &str)
    : Expr(loc, qt), value(str) {}

void ASTDumper::visitUnaryExpr(UnaryExpr *unary) {
}

void ASTDumper::visitBinaryExpr(BinaryExpr *binary) {
    out << "BinaryExpr <" << srcLocToPos(binary->srcLoc) << "> ";
    switch (binary->op) {
    case ADD:
        out << "'+'\n";
        break;
    case SUB:
        out << "'-'\n";
        break;
    case MUL:
        out << "'*'\n";
        break;
    case DIV:
        out << "'/'\n";
        break;
    case MOD:
        out << "'%'\n";
        break;
    case SHL:
        out << "'<<'\n";
        break;
    case SHR:
        out << "'>>'\n";
        break;
    case LESS:
        out << "'<'\n";
        break;
    case LEQ:
        out << "'<='\n";
        break;
    case GREATER:
        out << "'>'\n";
        break;
    case GEQ:
        out << "'>='\n";
        break;
    case EQUAL:
        out << "'=='\n";
        break;
    case NEQ:
        out << "'!='\n";
        break;
    case BITAND:
        out << "'&'\n";
        break;
    case BITXOR:
        out << "'^'\n";
        break;
    case BITOR:
        out << "'|'\n";
        break;
    case LOGICAND:
        out << "'&&'\n";
        break;
    case LOGICOR:
        out << "'||'\n";
        break;
    case ASSIGN:
        out << "'='\n";
        break;
    case COMMA:
        out << "','\n";
        break;
    default:
        break;
    }
    out << prefix << "|-- ";
    prefix.append("|   ");
    binary->lhs->accept(this);
    prefix.erase(prefix.size() - 4);

    out << prefix << "`-- ";
    prefix.append("    ");
    binary->rhs->accept(this);
    prefix.erase(prefix.size() - 4);
}

void ASTDumper::visitTernaryExpr(TernaryExpr *ternary) {
    out << "TernaryExpr <" << srcLocToPos(ternary->srcLoc) << ">\n";

    out << prefix << "|-- ";
    prefix.append("|   ");
    ternary->condExpr->accept(this);
    prefix.erase(prefix.size() - 4);

    out << prefix << "|-- ";
    prefix.append("|   ");
    ternary->trueExpr->accept(this);
    prefix.erase(prefix.size() - 4);

    out << prefix << "`-- ";
    prefix.append("    ");
    ternary->falseExpr->accept(this);
    prefix.erase(prefix.size() - 4);
}

void ASTDumper::visitIntegerConstant(IntegerConstant *integer) {
    auto repr_pair = integer->qtype.repr();
    out << "IntegerConstant <" << srcLocToPos(integer->srcLoc) << "> '";
    out << repr_pair.first << repr_pair.second << "' " << integer->value << '\n';
}

void ASTDumper::visitFloatingConstant(FloatingConstant *floating) {
    auto repr_pair = floating->qtype.repr();
    out << "FloatingConstant <" << srcLocToPos(floating->srcLoc) << "> '";
    out << repr_pair.first << repr_pair.second << "' " << floating->value << '\n';
}

void ASTDumper::visitCharacterConstant(CharacterConstant *character) {
    auto repr_pair = character->qtype.repr();
    out << "CharacterConstant <" << srcLocToPos(character->srcLoc) << "> '";
    out << repr_pair.first << repr_pair.second << "' " << character->value << '\n';
}

void ASTDumper::visitStringLiteral(StringLiteral *string) {
    auto repr_pair = string->qtype.repr();
    out << "StringLiteral <" << srcLocToPos(string->srcLoc) << "> '";
    out << repr_pair.first << repr_pair.second << "' \"" << string->value << "\"\n";
}
