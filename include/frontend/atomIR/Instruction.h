#ifndef ATOM_INSTRUCTION_H
#define ATOM_INSTRUCTION_H

#include "Function.h"

namespace ATC {
namespace AtomIR {

enum InstType {
    // special inst
    INST_ALLOC,
    INST_STORE,
    INST_FUNCALL,
    INST_RET,

    // unary inst
    INST_LOAD,
    INST_ITOF,
    INST_FTOI,

    // binary inst
    INST_ADD,
    INST_SUB,
    INST_MUL,
    INST_DIV,
    INST_MOD,
    INST_LT,
    INST_LE,
    INST_GT,
    INST_GE,
    INST_EQ,
    INST_NE,
    INST_AND,
    INST_OR,

    // jump inst
    INST_JUMP,
    INST_JLT,
    INST_JLE,
    INST_JGT,
    INST_JGE,
    INST_JEQ,
    INST_JNE,
};

enum InstId {
    ID_ALLOC_INST,
    ID_STORE_INST,
    ID_FUNCTION_CALL_INST,
    ID_RETURN_INST,
    ID_UNARY_INST,
    ID_BINARY_INST,
    ID_JUMP_INST,
    ID_COND_JUMP_INST
};

class Instruction {
public:
    Instruction(InstType type) : _type(type) {}
    int getInstType() { return _type; }

    virtual int getClassId() = 0;

    virtual std::string toString() = 0;

    virtual Value* getResult() { return nullptr; }

protected:
    InstType _type;
};

class AllocInst : public Instruction {
public:
    AllocInst(Type* allocType, const std::string& resultName = "");

    virtual int getClassId() override { return ID_ALLOC_INST; }

    virtual std::string toString() override;

    virtual Value* getResult() override { return _result; }

private:
    Value* _result;
};

class StoreInst : public Instruction {
public:
    StoreInst(Value* value, Value* dest) : Instruction(INST_STORE), _value(value), _dest(dest) {}

    virtual int getClassId() override { return ID_STORE_INST; }

    virtual std::string toString() override;

private:
    Value* _value;
    Value* _dest;
};

class FunctionCallInst : public Instruction {
public:
    FunctionCallInst(FunctionType functionType, const std::string& funcName, const std::vector<Value*>& params,
                     const std::string& resultName = "");

    virtual int getClassId() override { return ID_FUNCTION_CALL_INST; }

    virtual std::string toString() override;

    virtual Value* getResult() override { return _result; }

private:
    std::string _funcName;
    std::vector<Value*> _params;
    Value* _result;
};

class ReturnInst : public Instruction {
public:
    // retValue is nullptr when ret void
    ReturnInst(Value* retValue = nullptr) : Instruction(INST_RET), _retValue(retValue) {}

    virtual int getClassId() override { return ID_RETURN_INST; }

    virtual std::string toString() override;

private:
    Value* _retValue;
};

class UnaryInst : public Instruction {
public:
    UnaryInst(InstType type, Value* operand, const std::string& resultName = "");

    virtual int getClassId() override { return ID_UNARY_INST; }

    virtual std::string toString() override;

    virtual Value* getResult() override { return _result; }

private:
    Value* _operand;
    Value* _result;
};

class BinaryInst : public Instruction {
public:
    BinaryInst(InstType type, Value* operand1, Value* operand2, const std::string& resultName = "");

    virtual int getClassId() override { return ID_BINARY_INST; }

    virtual std::string toString() override;

    virtual Value* getResult() override { return _result; }

    bool isIntInst() { return _intInst; }

private:
    Value* _operand1;
    Value* _operand2;
    Value* _result;
    bool _intInst;
};

class JumpInst : public Instruction {
public:
    JumpInst(BasicBlock* targetBB) : Instruction(INST_JUMP), _targetBB(targetBB) {}

    virtual int getClassId() override { return ID_JUMP_INST; }

    virtual std::string toString() override;

private:
    BasicBlock* _targetBB;
};

class CondJumpInst : public Instruction {
public:
    CondJumpInst(InstType type, BasicBlock* trueBB, BasicBlock* falseBB, Value* operand1, Value* operand2);

    virtual int getClassId() override { return ID_COND_JUMP_INST; }

    virtual std::string toString() override;

    bool isIntInst() { return _intInst; }

private:
    BasicBlock* _trueBB;
    BasicBlock* _falseBB;
    Value* _operand1;
    Value* _operand2;
    bool _intInst;
};

}  // namespace AtomIR
}  // namespace ATC

#endif