#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <vector>

#include "TreeNode.h"
#include "Variable.h"
namespace ATC {
class Expression : public TreeNode {
public:
    Expression() = default;
    virtual int getClassId() override { return ID_EXPRESSION; }
    virtual bool isConst() { return false; }
};

class ConstVal : public Expression {
public:
    ConstVal() = default;
    virtual int getClassId() override { return ID_CONST_VAL; }

    int getDataType() { return _type; }
    virtual bool isConst() override { return true; }
    int getIntValue() { return _intValue; }
    float getFloatValue() { return _floatValue; }

    void setDataType(DataType type) { _type = type; }
    void setIntValue(int value) { _intValue = value; }
    void setFloatValue(float value) { _floatValue = value; }

private:
    DataType _type;
    int _intValue = 0;
    float _floatValue = 0;
};

class VarRef : public Expression {
public:
    virtual int getClassId() override { return ID_VAR_REF; }
    virtual bool isConst() { return _var->isConst(); }

    Variable* getVariable() { return _var; }
    const std::vector<Expression*>& getDimension() { return _dimensions; }

    void setVariable(Variable* var) { _var = var; }
    void addDimension(Expression* dimension) { _dimensions.push_back(dimension); }

private:
    Variable* _var;
    std::vector<Expression*> _dimensions;
};

class UnaryExpression : public Expression {
public:
    UnaryExpression() = default;

    virtual int getClassId() override { return ID_UNARY_EXPRESSION; }
    virtual bool isConst() { return _operand->isConst(); }

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
    virtual bool isConst() { return _left->isConst() && _right->isConst(); }

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