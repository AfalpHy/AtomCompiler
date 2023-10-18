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

    DataType* getDataType() { return _dataType; }
    const std::vector<Variable*>& getVariables() { return _vars; }

    void setDataType(DataType* dataType) { _dataType = dataType; }
    void addVariable(Variable* var) { _vars.push_back(var); }

    ACCEPT
private:
    DataType* _dataType;
    std::vector<Variable*> _vars;
};

}  // namespace ATC

#endif