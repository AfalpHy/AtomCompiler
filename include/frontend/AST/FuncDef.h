#ifndef FUNC_DEF_H
#define FUNC_DEF_H

#include <vector>

#include "Decl.h"
#include "Statement.h"
namespace ATC {
class FuncDef : public TreeNode {
public:
    FuncDef(/* args */) = default;
    virtual int getClassId() override { return ID_FUNC_DEF; }
    ~FuncDef() = default;

    DataType getRetType() { return _retType; }
    const std::vector<Decl*>& getParams() { return _params; }
    Block* getBlock() { return _block; }

    void setRetType(DataType retType) { _retType = retType; }
    void addParams(Decl* decl) { _params.push_back(decl); }
    void setBlock(Block* block) { _block = block; }

    ACCEPT

private:
    DataType _retType;
    std::vector<Decl*> _params;
    Block* _block;
};

}  // namespace ATC

#endif