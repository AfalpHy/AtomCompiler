#pragma once

#include <vector>

#include "TreeNode.h"
#include "Variable.h"

namespace ATC {

class Decl : public TreeNode {
public:
    Decl() = default;
};

class VarDecl : public Decl {
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
