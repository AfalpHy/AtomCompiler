#ifndef FUNC_DEF_H
#define FUNC_DEF_H

#include <vector>

#include "Statement.h"

namespace ATC {
class FuncDef : public Node {
public:
    FuncDef(/* args */) = default;
    ~FuncDef() = default;

private:
    DataType _retType;
    std::vector<Expression*> _params;
    Block* _block;
};

}  // namespace ATC

#endif