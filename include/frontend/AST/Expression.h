#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <vector>

#include "Node.h"
namespace ATC {
class Expression : public Node {
public:
    Expression() = default;
    virtual int getClassId() override { return ID_EXPRESSION; }
};

class UnaryExpression : public Expression {
public:
    UnaryExpression() = default;

    virtual int getClassId() override { return ID_UNARY_EXPRESSION; }

    Operator getOperator() { return _operator; }
    Expression* getOperand() { return _operand; }

    void setOperator(Operator op) { _operator = op; }
    void setOperand(Expression* operand) { _operand = operand; }

private:
    Operator _operator;
    Expression* _operand;
};

class BinaryExpression : public Expression {
public:
    BinaryExpression() = default;

    virtual int getClassId() override { return ID_BINARY_EXPRESSION; }
    Operator getOperator() { return _operator; }
    Expression* getLeft() { return _left; }
    Expression* getRight() { return _right; }

    void setOperator(Operator op) { _operator = op; }
    void setLeft(Expression* left) { _left = left; }
    void setRight(Expression* right) { _right = right; }

private:
    Operator _operator;
    Expression* _left;
    Expression* _right;
};

}  // namespace ATC
#endif