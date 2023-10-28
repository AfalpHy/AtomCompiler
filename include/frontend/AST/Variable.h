#ifndef VARIABLE_H
#define VARIABLE_H

#include <llvm/IR/Value.h>

#include "DataType.h"
namespace ATC {

class Expression;
class Variable : public TreeNode {
public:
    Variable() = default;
    Variable(TreeNode* parent) : TreeNode(parent) {}

    virtual int getClassId() { return ID_VARIABLE; }

    int getBasicType();
    
    DataType* getDataType() { return _dataType; }
    Expression* getInitValue() { return _initValue; }
    bool isConst() { return _isConst; }
    bool isGlobal() { return _isGlobal; }
    llvm::Value* getAddr() { return _addr; }

    void setDataType(DataType* dataType) { _dataType = dataType; }
    void setInitValue(Expression* value) { _initValue = value; }
    void setIsConst(bool b) { _isConst = b; }
    void setIsGlobal(bool b) { _isGlobal = b; }
    void setAddr(llvm::Value* addr) { _addr = addr; }

    ACCEPT

private:
    DataType* _dataType;
    Expression* _initValue = nullptr;
    bool _isConst = false;
    bool _isGlobal = false;
    llvm::Value* _addr;  // the pointer of var in llvmIR
};

}  // namespace ATC

#endif