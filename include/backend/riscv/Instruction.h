#pragma once

#include <string>

#include "Register.h"

namespace ATC {
namespace RISCV_ARCH {

class BasicBlock;

enum ByteLen { BYTE, HALF_WORD, WORD, DOUBLE_WORD };

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
    virtual int getClassId() = 0;

    virtual std::string toString() = 0;

    virtual Register* getResult() { return nullptr; }

protected:
    ByteLen _len;
};

class StoreInst : public Instruction {
public:
    StoreInst(Register* value, Register* dest, int imm, ByteLen len) : _value(value), _dest(dest), _imm(imm) {
        _len = len;
    }

    virtual std::string toString() override;

private:
    Register* _value;
    Register* _dest;
    int _imm;
};

class FunctionCallInst : public Instruction {
public:
    FunctionCallInst(const std::string& funcName);

    virtual std::string toString() override { return "call\t" + _funcName; }

private:
    std::string _funcName;
};

class ReturnInst : public Instruction {
public:
    virtual std::string toString() override { return "ret"; }
};

class UnaryInst : public Instruction {
public:
    UnaryInst(int type, Register* operand, const std::string& resultName = "");

    virtual std::string toString() override;

    virtual Register* getResult() override { return _result; }

private:
    Register* _operand;
    Register* _result;
    int _imm;
};

class BinaryInst : public Instruction {
public:
private:
    Register* _result;
    Register* _operand1;
    Register* _opreand2 = nullptr;
    int _imm;
};

class JumpInst : public Instruction {
public:
    JumpInst(const std::string& targetBB) : _targetBB(targetBB) {}

    virtual int getClassId() override { return ID_JUMP_INST; }

    virtual std::string toString() override;

private:
    std::string _targetBB;
};

class CondJumpInst : public JumpInst {
public:
    CondJumpInst(int type, const std::string& targetBB);

    virtual std::string toString() override;

private:
    std::string _targetBB;
};

}  // namespace RISCV_ARCH
}  // namespace ATC
