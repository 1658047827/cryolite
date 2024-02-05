#include "ast.h"

UnaryExpr::UnaryExpr(const SourceLocation &loc, UnaryOpKind op, Expr *expr)
    : Expr(loc), op(op), operand(expr) {}

BinaryExpr::BinaryExpr(const SourceLocation &loc, BinaryOpKind op, Expr *lhs, Expr *rhs)
    : Expr(loc), op(op), lhs(lhs), rhs(rhs) {}

TernaryExpr::TernaryExpr(const SourceLocation &loc, Expr *condExpr, Expr *trueExpr, Expr *falseExpr)
    : Expr(loc), condExpr(condExpr), trueExpr(trueExpr), falseExpr(falseExpr) {}

IntegerConstant::IntegerConstant(const SourceLocation &loc, unsigned long long val)
    : Expr(loc), value(val) {}

FloatingConstant::FloatingConstant(const SourceLocation &loc, double val)
    : Expr(loc), value(val) {}

StringLiteral::StringLiteral(const SourceLocation &loc, const std::string &str)
    : Expr(loc), value(str) {}

void ASTDump::visitUnaryExpr(UnaryExpr *unary) {
}

void ASTDump::visitBinaryExpr(BinaryExpr *binary) {
    out << "BinaryExpr ";
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
    out << prefix << "├── ";
    prefix.append(u);
    binary->lhs->accept(this);
    prefix.erase(prefix.size() - u.size());

    out << prefix << "└── ";
    prefix.append("    ");
    binary->rhs->accept(this);
    prefix.erase(prefix.size() - 4);
}

void ASTDump::visitTernaryExpr(TernaryExpr *ternary) {
    out << "TernaryExpr\n";

    out << prefix << "├── ";
    prefix.append(u);
    ternary->condExpr->accept(this);
    prefix.erase(prefix.size() - u.size());

    out << prefix << "├── ";
    prefix.append(u);
    ternary->trueExpr->accept(this);
    prefix.erase(prefix.size() - u.size());

    out << prefix << "└── ";
    prefix.append("    ");
    ternary->trueExpr->accept(this);
    prefix.erase(prefix.size() - 4);
}

void ASTDump::visitIntegerConstant(IntegerConstant *integer) {
    out << "IntegerConstant " << integer->value << '\n';
}

void ASTDump::visitFloatingConstant(FloatingConstant *floating) {}
