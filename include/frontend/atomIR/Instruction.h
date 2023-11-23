#ifndef ATOM_INSTRUCTION_H
#define ATOM_INSTRUCTION_H

#include "BasicBlock.h"
#include "Value.h"

namespace ATC {
namespace AtomIR {

enum InstType {
    // special inst
    INST_ALLOC,
    INST_STORE,
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

class Instruction {
public:
    Instruction(InstType type) : _type(type) {}
    int getInstType() { return _type; }

    virtual std::string toString() = 0;

    virtual Value* getResult() { return nullptr; }

protected:
    InstType _type;
};

class AllocInst : public Instruction {
public:
    AllocInst(Type* allocType, const std::string& resultName = "");

    virtual std::string toString() override;

    virtual Value* getResult() override { return _result; }

private:
    Value* _result;
};

class StoreInst : public Instruction {
public:
    StoreInst(Value* value, Value* dest) : Instruction(INST_STORE), _value(value), _dest(dest) {}

    virtual std::string toString() override;

private:
    Value* _value;
    Value* _dest;
};

class ReturnInst : public Instruction {
public:
    ReturnInst(Value* retValue) : Instruction(INST_RET), _retValue(retValue) {}

    virtual std::string toString() override;

private:
    Value* _retValue;
};

class UnaryInst : public Instruction {
public:
    UnaryInst(InstType type, Value* operand, const std::string& resultName = "");

    virtual std::string toString() override;

    virtual Value* getResult() override { return _result; }

private:
    Value* _operand;
    Value* _result;
};

class BinaryInst : public Instruction {
public:
    BinaryInst(InstType type, Value* operand1, Value* operand2, const std::string& resultName = "");

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

    virtual std::string toString() override;

private:
    BasicBlock* _targetBB = nullptr;
};

class CondJumpInst : public Instruction {
public:
    CondJumpInst(InstType type, BasicBlock* trueBB, BasicBlock* falseBB, Value* operand1, Value* operand2);

    virtual std::string toString() override;

    bool isIntInst() { return _intInst; }

private:
    BasicBlock* _trueBB = nullptr;
    BasicBlock* _falseBB = nullptr;
    Value* _operand1 = nullptr;
    Value* _operand2 = nullptr;
    bool _intInst;
};

}  // namespace AtomIR
}  // namespace ATC

#endif