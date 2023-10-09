#ifndef DECL_H
#define DECL_H

#include <vector>

#include "TreeNode.h"
#include "Variable.h"
namespace ATC {
class Decl : public TreeNode {
public:
    Decl(/* args */) = default;
    ~Decl() = default;
    virtual int getClassId() { return ID_DECL; }

    Variable* getVariable() { return _var; }

    void setVariable(Variable* var) { _var = var; }

private:
    Variable* _var;
};

}  // namespace ATC

#endif