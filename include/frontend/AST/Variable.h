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

    Expression* getInitValue() { return _initValue; }
    bool isConst() { return _isConst; }
    bool isGlobal() { return _isGlobal; }
    llvm::Value* getAddr() { return _addr; }
    DataType* getDataType();

    void setInitValue(Expression* value) { _initValue = value; }
    void setIsConst(bool b) { _isConst = b; }
    void setIsGlobal(bool b) { _isGlobal = b; }
    void setAddr(llvm::Value* addr) { _addr = addr; }

    ACCEPT

private:
    Expression* _initValue = nullptr;
    bool _isConst = false;
    bool _isGlobal = false;
    llvm::Value* _addr;  // llvmIR中保存该变量的地址指针
};

}  // namespace ATC

#endif