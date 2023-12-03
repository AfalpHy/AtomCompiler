#ifndef ATC_VARIABLE_H
#define ATC_VARIABLE_H

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
    llvm::Value* getLLVMAddr() { return _llvmAddr; }
    AtomIR::Value* getAtomAddr() { return _atomAddr; }

    void setDataType(DataType* dataType) { _dataType = dataType; }
    void setInitValue(Expression* value) { _initValue = value; }
    void setIsConst(bool b) { _isConst = b; }
    void setIsGlobal(bool b) { _isGlobal = b; }
    void setLLVMAddr(llvm::Value* addr) { _llvmAddr = addr; }
    void setAtomAddr(AtomIR::Value* addr) { _atomAddr = addr; }

    ACCEPT

private:
    DataType* _dataType;
    Expression* _initValue = nullptr;
    bool _isConst = false;
    bool _isGlobal = false;
    llvm::Value* _llvmAddr = nullptr;  // the pointer of var in llvmIR
    AtomIR::Value* _atomAddr = nullptr;
};

}  // namespace ATC

#endif