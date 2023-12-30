#pragma once

#include <vector>

#include "Value.h"

namespace ATC {
namespace AtomIR {

struct FunctionType;

class BasicBlock;

enum InstId {
    ID_ALLOC_INST,
    ID_STORE_INST,
    ID_FUNCTION_CALL_INST,
    ID_GETELEMENTPTR_INST,
    ID_BITCAST_INST,
    ID_RETURN_INST,
    ID_UNARY_INST,
    ID_BINARY_INST,
    ID_JUMP_INST,
    ID_COND_JUMP_INST
};

class Instruction {
public:
    virtual int getClassId() = 0;

    virtual std::string toString() = 0;

    virtual Value* getResult() { return nullptr; }
};

class AllocInst : public Instruction {
public:
    AllocInst(Type* allocType, const std::string& resultName = "");

    virtual int getClassId() override { return ID_ALLOC_INST; }

    virtual std::string toString() override;

    virtual Value* getResult() override { return _result; }

    bool isAllocForParam() { return _allocForParam; }

    int getAllocatedIntParamNum() { return _allocatedIntParamNum; }

    int getAllocatedFloatParamNum() { return _allocatedFloatParamNum; }

    static bool AllocForParam;
    static int AllocatedIntParamNum;
    static int AllocatedFloatParamNum;

private:
    Value* _result;
    bool _allocForParam;
    int _allocatedIntParamNum = -1;
    int _allocatedFloatParamNum = -1;
};

class StoreInst : public Instruction {
public:
    StoreInst(Value* value, Value* dest) : _value(value), _dest(dest) {}

    virtual int getClassId() override { return ID_STORE_INST; }

    virtual std::string toString() override;

    Value* getValue() { return _value; }

    Value* getDest() { return _dest; }

private:
    Value* _value;
    Value* _dest;
};

class FunctionCallInst : public Instruction {
public:
    FunctionCallInst(const FunctionType& functionType, const std::string& funcName, const std::vector<Value*>& params,
                     const std::string& resultName = "");

    virtual int getClassId() override { return ID_FUNCTION_CALL_INST; }

    virtual std::string toString() override;

    virtual Value* getResult() override { return _result; }

    const std::string& getFuncName() { return _funcName; }

    const std::vector<Value*>& getParams() { return _params; }

private:
    std::string _funcName;
    std::vector<Value*> _params;
    Value* _result;
};

class GetElementPtrInst : public Instruction {
public:
    GetElementPtrInst(Value* ptr, const std::vector<Value*>& indexes, const std::string& resultName = "");

    virtual int getClassId() override { return ID_GETELEMENTPTR_INST; }

    virtual std::string toString() override;

    virtual Value* getResult() override { return _result; }

    Value* getPtr() { return _ptr; }

    const std::vector<Value*>& getIndexes() { return _indexes; }

private:
    Value* _ptr;
    std::vector<Value*> _indexes;
    Value* _result;
};

class BitCastInst : public Instruction {
public:
    BitCastInst(Value* ptr, Type* destTy);

    virtual int getClassId() override { return ID_BITCAST_INST; }

    virtual std::string toString() override;

    virtual Value* getResult() override { return _result; }

    Value* getPtr() { return _ptr; }

private:
    Value* _ptr;
    Value* _result;
};

class ReturnInst : public Instruction {
public:
    // retValue is nullptr when ret void
    ReturnInst(Value* retValue = nullptr) : _retValue(retValue) {}

    virtual int getClassId() override { return ID_RETURN_INST; }

    virtual std::string toString() override;

private:
    Value* _retValue;
};

class UnaryInst : public Instruction {
public:
    UnaryInst(int type, Value* operand, const std::string& resultName = "");

    virtual int getClassId() override { return ID_UNARY_INST; }

    virtual std::string toString() override;

    virtual Value* getResult() override { return _result; }

    Value* getOperand() { return _operand; }

    int getInstType() { return _type; }

    enum { INST_LOAD, INST_ITOF, INST_FTOI };

private:
    Value* _operand;
    Value* _result;
    int _type;
};

class BinaryInst : public Instruction {
public:
    BinaryInst(int type, Value* operand1, Value* operand2, const std::string& resultName = "");

    virtual int getClassId() override { return ID_BINARY_INST; }

    virtual std::string toString() override;

    virtual Value* getResult() override { return _result; }

    Value* getOperand1() { return _operand1; }

    Value* getOperand2() { return _operand2; }

    bool isIntInst() { return _operand1->getType() == Type::getInt32Ty(); }

    int getInstType() { return _type; }

    enum {
        INST_ADD,
        INST_SUB,
        INST_MUL,
        INST_DIV,
        INST_MOD,
        INST_BIT_AND,
        INST_BIT_OR,
        INST_LT,
        INST_LE,
        INST_GT,
        INST_GE,
        INST_EQ,
        INST_NE
    };

private:
    Value* _operand1;
    Value* _operand2;
    Value* _result;
    int _type;
};

class JumpInst : public Instruction {
public:
    JumpInst(BasicBlock* targetBB) : _targetBB(targetBB) {}

    virtual int getClassId() override { return ID_JUMP_INST; }

    virtual std::string toString() override;

    BasicBlock* getTargetBB() { return _targetBB; }

private:
    BasicBlock* _targetBB;
};

class CondJumpInst : public Instruction {
public:
    CondJumpInst(int type, BasicBlock* trueBB, BasicBlock* falseBB, Value* operand1, Value* operand2);

    virtual int getClassId() override { return ID_COND_JUMP_INST; }

    virtual std::string toString() override;

    BasicBlock* getTureBB() { return _trueBB; }

    BasicBlock* getFalseBB() { return _falseBB; }

    Value* getOperand1() { return _operand1; }

    Value* getOperand2() { return _operand2; }

    bool isIntInst() { return _operand1->getType() == Type::getInt32Ty(); }

    int getInstType() { return _type; }

    enum { INST_JLT, INST_JLE, INST_JGT, INST_JGE, INST_JEQ, INST_JNE };

private:
    BasicBlock* _trueBB;
    BasicBlock* _falseBB;
    Value* _operand1;
    Value* _operand2;
    int _type;
};

}  // namespace AtomIR
}  // namespace ATC
