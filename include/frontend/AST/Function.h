#pragma once

#include <vector>

#include "Statement.h"
namespace ATC {

class FunctionDecl : public TreeNode {
public:
    FunctionDecl() = default;

    virtual int getClassId() override { return ID_FUNCTION_DEF; }

    DataType* getRetType() { return _retType; }
    const std::vector<VarDecl*>& getParams() { return _params; }

    void setRetType(DataType* retType) { _retType = retType; }
    void addParams(VarDecl* decl) { _params.push_back(decl); }

    ACCEPT

private:
    DataType* _retType;
    std::vector<VarDecl*> _params;
};

class FunctionDef : public TreeNode {
public:
    FunctionDef(FunctionDecl* decl) : _functionDecl(decl) {}

    virtual int getClassId() override { return ID_FUNCTION_DEF; }

    FunctionDecl* getFunctionDecl() { return _functionDecl; }

    Block* getBlock() { return _block; }

    void setBlock(Block* block) { _block = block; }

    ACCEPT

private:
    FunctionDecl* _functionDecl;
    Block* _block = nullptr;
};

}  // namespace ATC
