#ifndef ATC_FUNCTION_DEF_H
#define ATC_FUNCTION_DEF_H

#include <llvm/IR/Function.h>

#include <vector>

#include "Decl.h"
#include "Statement.h"
namespace ATC {

namespace AtomIR {
class Function;
}

class FunctionDef : public TreeNode {
public:
    FunctionDef() = default;
    FunctionDef(TreeNode* parent) : TreeNode(parent) {}

    virtual int getClassId() override { return ID_FUNCTION_DEF; }

    DataType* getRetType() { return _retType; }
    const std::vector<VarDecl*>& getParams() { return _params; }
    Block* getBlock() { return _block; }
    llvm::Function* getLLVMFunction() { return _llvmFunction; }
    AtomIR::Function* getAtomFunction() { return _atomFunction; }

    void setRetType(DataType* retType) { _retType = retType; }
    void addParams(VarDecl* decl) { _params.push_back(decl); }
    void setBlock(Block* block) { _block = block; }
    void setLLVMFunction(llvm::Function* function) { _llvmFunction = function; }
    void setAtomFunction(AtomIR::Function* function) { _atomFunction = function; }

    ACCEPT

private:
    DataType* _retType;
    std::vector<VarDecl*> _params;
    Block* _block;
    llvm::Function* _llvmFunction = nullptr;
    AtomIR::Function* _atomFunction = nullptr;
};

}  // namespace ATC

#endif