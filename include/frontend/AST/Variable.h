#ifndef VARIABLE_H
#define VARIABLE_H

#include <llvm/IR/Value.h>

#include "DataType.h"
namespace ATC {

class Expression;
class Variable : public TreeNode {
public:
    Variable() = default;
    virtual int getClassId() { return ID_VARIABLE; }

    DataType* getDataType() { return _type; }
    Expression* getInitValue() { return _initValue; }
    bool isConst() { return _isConst; }
    llvm::Value* getAddr() { return _addr; }

    void setDataType(DataType* type) { _type = type; }
    void setInitValue(Expression* value) { _initValue = value; }
    void setIsConst() { _isConst = true; }
    void setAddr(llvm::Value* addr) { _addr = addr; }

    ACCEPT

private:
    DataType* _type;
    Expression* _initValue = nullptr;
    bool _isConst = false;
    llvm::Value* _addr;  // llvmIR中保存该变量的地址指针
};

}  // namespace ATC

#endif