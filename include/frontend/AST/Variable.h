#pragma once

#include "DataType.h"
namespace ATC {

class Expression;

class Variable : public TreeNode {
public:
    Variable() = default;

    virtual int getClassId() { return ID_VARIABLE; }

    int getBasicType();

    DataType* getDataType() { return _dataType; }
    Expression* getInitValue() { return _initValue; }
    bool isConst() { return _isConst; }
    bool isGlobal() { return _isGlobal; }

    void setDataType(DataType* dataType) { _dataType = dataType; }
    void setInitValue(Expression* value) { _initValue = value; }
    void setIsConst(bool b) { _isConst = b; }
    void setIsGlobal(bool b) { _isGlobal = b; }

    ACCEPT

private:
    DataType* _dataType = nullptr;
    Expression* _initValue = nullptr;
    bool _isConst = false;
    bool _isGlobal = false;
};

class VarDecl : public TreeNode {
public:
    VarDecl() = default;

    virtual int getClassId() { return ID_VAR_DECL; }

    DataType* getDataType() { return _declType; }
    const std::vector<Variable*>& getVariables() { return _vars; }

    void setDataType(DataType* declType) { _declType = declType; }
    void addVariable(Variable* var) { _vars.push_back(var); }

    ACCEPT

private:
    DataType* _declType;
    std::vector<Variable*> _vars;
};

}  // namespace ATC
