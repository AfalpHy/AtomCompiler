#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <vector>

#include "TreeNode.h"
#include "Variable.h"
namespace ATC {
class Expression : public TreeNode {
public:
    Expression() = default;
    Expression(TreeNode* parent) : TreeNode(parent) {}

    virtual bool isConst() = 0;
    static float evaluateConstExpr(Expression* expr);

    int getBaseType() { return _baseType; }
    bool isCond() { return _isCond; }

    void setBaseType(BaseType baseType) { _baseType = baseType; }
    void setIsCond(bool b) { _isCond = b; }

    static void setCond(Expression* expr);

protected:
    BaseType _baseType;
    bool _isCond = false;
};

class ConstVal : public Expression {
public:
    ConstVal() = default;
    ConstVal(TreeNode* parent) : Expression(parent) {}

    virtual int getClassId() override { return ID_CONST_VAL; }

    virtual bool isConst() override { return true; }
    int getIntValue() { return _intValue; }
    float getFloatValue() { return _floatValue; }

    void setIntValue(int value) { _intValue = value; }
    void setFloatValue(float value) { _floatValue = value; }

    ACCEPT

private:
    int _intValue = 0;
    float _floatValue = 0;
};

class VarRef : public Expression {
public:
    VarRef() = default;
    VarRef(TreeNode* parent) : Expression(parent) {}

    virtual int getClassId() override { return ID_VAR_REF; }
    virtual bool isConst() { return _var->isConst(); }

    Variable* getVariable() { return _var; }
    const std::vector<Expression*>& getDimensions() { return _dimensions; }

    void setVariable(Variable* var) { _var = var; }
    void addDimension(Expression* dimension) { _dimensions.push_back(dimension); }

    ACCEPT

private:
    Variable* _var;
    std::vector<Expression*> _dimensions;
};

class ArrayExpression : public Expression {
public:
    ArrayExpression() = default;
    ArrayExpression(TreeNode* parent) : Expression(parent) {}

    virtual int getClassId() override { return ID_ARRAY_EXPRESSION; }

    virtual bool isConst();

    const std::vector<Expression*>& getElements() { return _elements; }

    void addElement(Expression* element) { _elements.push_back(element); }

    ACCEPT

private:
    std::vector<Expression*> _elements;
};

class UnaryExpression : public Expression {
public:
    UnaryExpression() = default;
    UnaryExpression(TreeNode* parent) : Expression(parent) {}

    virtual int getClassId() override { return ID_UNARY_EXPRESSION; }
    virtual bool isConst() { return _operand->isConst(); }

    Operator getOperator() { return _operator; }
    Expression* getOperand() { return _operand; }

    void setOperator(Operator op) { _operator = op; }
    void setOperand(Expression* operand) { _operand = operand; }

    ACCEPT

private:
    Operator _operator;
    Expression* _operand;
};

class BinaryExpression : public Expression {
public:
    BinaryExpression() = default;
    BinaryExpression(TreeNode* parent) : Expression(parent) {}

    virtual int getClassId() override { return ID_BINARY_EXPRESSION; }
    virtual bool isConst() { return _left->isConst() && _right->isConst(); }

    Operator getOperator() { return _operator; }
    Expression* getLeft() { return _left; }
    Expression* getRight() { return _right; }

    void setOperator(Operator op) { _operator = op; }
    void setLeft(Expression* left) { _left = left; }
    void setRight(Expression* right) { _right = right; }

    ACCEPT

private:
    Operator _operator;
    Expression* _left;
    Expression* _right;
};

class FunctionCall : public Expression {
public:
    FunctionCall() = default;
    FunctionCall(TreeNode* parent) : Expression(parent) {}

    virtual int getClassId() override { return ID_FUNCTION_CALL; }
    virtual bool isConst() override { return false; }

    const std::vector<Expression*>& getParams() { return _params; }
    FunctionDef* getFunctionDef() { return _functionDef; }

    void addParams(Expression* param) { _params.push_back(param); }
    void setFunctionDef(FunctionDef* functionDef) { _functionDef = functionDef; }

    ACCEPT

private:
    std::vector<Expression*> _params;
    FunctionDef* _functionDef;
};
}  // namespace ATC
#endif