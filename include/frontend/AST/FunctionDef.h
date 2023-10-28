#ifndef FUNCTION_DEF_H
#define FUNCTION_DEF_H

#include <llvm/IR/Function.h>

#include <vector>

#include "Decl.h"
#include "Statement.h"
namespace ATC {

class FunctionDef : public TreeNode {
public:
    FunctionDef() = default;
    FunctionDef(TreeNode* parent) : TreeNode(parent) {}

    virtual int getClassId() override { return ID_FUNCTION_DEF; }

    DataType* getRetType() { return _retType; }
    const std::vector<VarDecl*>& getParams() { return _params; }
    Block* getBlock() { return _block; }
    llvm::Function* getFunction() { return _function; }

    void setRetType(DataType* retType) { _retType = retType; }
    void addParams(VarDecl* decl) { _params.push_back(decl); }
    void setBlock(Block* block) { _block = block; }
    void setFunction(llvm::Function* function) { _function = function; }

    ACCEPT

private:
    DataType* _retType;
    std::vector<VarDecl*> _params;
    Block* _block;
    llvm::Function* _function;
};

}  // namespace ATC

#endif