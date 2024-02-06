#ifndef _CRYOLITE_VISITOR_H_
#define _CRYOLITE_VISITOR_H_

class UnaryExpr;
class BinaryExpr;
class TernaryExpr;
class IntegerConstant;
class FloatingConstant;
class StringLiteral;

class Visitor {
public:
    virtual void visitUnaryExpr(UnaryExpr *unary) = 0;
    virtual void visitBinaryExpr(BinaryExpr *binary) = 0;
    virtual void visitTernaryExpr(TernaryExpr *ternary) = 0;
    virtual void visitIntegerConstant(IntegerConstant *integer) = 0;
    virtual void visitFloatingConstant(FloatingConstant *floating) = 0;
    virtual void visitStringLiteral(StringLiteral *string) = 0;
};

#endif