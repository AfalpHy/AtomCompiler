#pragma once

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

    virtual int getClassId() override { return ID_FUNCTION_DEF; }

    DataType* getRetType() { return _retType; }
    const std::vector<VarDecl*>& getParams() { return _params; }
    Block* getBlock() { return _block; }

    void setRetType(DataType* retType) { _retType = retType; }
    void addParams(VarDecl* decl) { _params.push_back(decl); }
    void setBlock(Block* block) { _block = block; }

    ACCEPT

private:
    DataType* _retType;
    std::vector<VarDecl*> _params;
    Block* _block;
};

}  // namespace ATC
