#ifndef VARIABLE_H
#define VARIABLE_H

#include <llvm/IR/Value.h>

#include "DataType.h"
namespace ATC {

class Expression;
namespace AtomIR {
class Value;
}

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

    void setAtomAddr(AtomIR::Value* addr) { _atomAddr = addr; }
    AtomIR::Value* getAtomAddr() { return _atomAddr; }
    ACCEPT

private:
    DataType* _dataType;
    Expression* _initValue = nullptr;
    bool _isConst = false;
    bool _isGlobal = false;
    llvm::Value* _addr;  // the pointer of var in llvmIR
    AtomIR::Value* _atomAddr;
};

}  // namespace ATC

#endif