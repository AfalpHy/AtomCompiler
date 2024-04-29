#pragma once

#include <vector>

#include "TreeNode.h"
#include "Variable.h"
namespace ATC {

enum Operator { PLUS, MINUS, NOT, MUL, DIV, MOD, LT, GT, LE, GE, EQ, NE, AND, OR };

class Expression : public TreeNode {
public:
    Expression() = default;

    virtual bool isConst() = 0;

protected:
};

class ConstVal : public Expression {
public:
    ConstVal() = default;

    virtual int getClassId() override { return ID_CONST_VAL; }

    virtual bool isConst() override { return true; }

    int getBasicType() { return _basicType; }
    int getIntValue() { return _intValue; }
    float getFloatValue() { return _floatValue; }

    void setBasicType(BasicType::Type basicType) { _basicType = basicType; }
    void setIntValue(int value) { _intValue = value; }
    void setFloatValue(float value) { _floatValue = value; }

    ACCEPT

private:
    BasicType::Type _basicType;
    int _intValue = 0;
    float _floatValue = 0;
};

class VarRef : public Expression {
public:
    VarRef() = default;

    virtual int getClassId() override { return ID_VAR_REF; }
    virtual bool isConst() { return _var->isConst(); }

    Variable* getVariable() { return _var; }

    void setVariable(Variable* var) { _var = var; }

    ACCEPT

private:
    Variable* _var;
};

class IndexedRef : public Expression {
public:
    IndexedRef() = default;

    virtual int getClassId() override { return ID_INDEXED_REF; }

    virtual bool isConst() {
        if (_var->isConst()) {
            if (_dimensions.size() != static_cast<ArrayType*>(_var->getDataType())->getDimensions().size()) {
                return false;
            }
            for (auto dim : _dimensions) {
                if (!dim->isConst()) {
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    Variable* getVariable() { return _var; }
    const std::vector<Expression*>& getDimensions() { return _dimensions; }

    void setVariable(Variable* var) { _var = var; }
    void addDimension(Expression* dimension) { _dimensions.push_back(dimension); }

    ACCEPT

private:
    Variable* _var;
    std::vector<Expression*> _dimensions;
};

class NestedExpression : public Expression {
public:
    NestedExpression() = default;

    virtual int getClassId() override { return ID_NESTED_EXPRESSION; }

    virtual bool isConst() {
        for (auto expr : _elements) {
            if (!expr->isConst()) {
                return false;
            }
        }
        return true;
    }

    Variable* getVariable() { return _var; }
    const std::vector<Expression*>& getElements() { return _elements; }

    void setVariable(Variable* var) { _var = var; }
    void addElement(Expression* element) { _elements.push_back(element); }

    ACCEPT

private:
    std::vector<Expression*> _elements;
    Variable* _var = nullptr;
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

    ACCEPT

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
    bool isForValue() { return _forValue; }

    void setOperator(Operator op) { _operator = op; }
    void setLeft(Expression* left) { _left = left; }
    void setRight(Expression* right) { _right = right; }
    void setNotForValue() { _forValue = false; }

    ACCEPT

private:
    Operator _operator;
    Expression* _left;
    Expression* _right;
    bool _forValue = true;
};

class FunctionCall : public Expression {
public:
    FunctionCall() = default;

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

class ExpressionHandle {
public:
    static bool isIntExpr(Expression* expr);
    static int evaluateConstIntExpr(Expression* expr);
    static float evaluateConstFloatExpr(Expression* expr);
};
}  // namespace ATC
