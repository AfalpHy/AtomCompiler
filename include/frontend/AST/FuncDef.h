#ifndef FUNC_DEF_H
#define FUNC_DEF_H

#include <vector>

#include "Decl.h"
#include "Statement.h"
namespace ATC {
class FuncDef : public TreeNode {
public:
    FuncDef(/* args */) = default;
    ~FuncDef() = default;

private:
    DataType _retType;
    std::vector<Decl*> _params;
    Block* _block;
};

}  // namespace ATC

#endif