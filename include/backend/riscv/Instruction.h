#pragma once

#include <string>

#include "Register.h"

namespace ATC {
namespace RISCV_ARCH {

class BasicBlock;

enum ByteLen { BYTE, HALF_WORD, WORD, DOUBLE_WORD };

class Instruction {
public:
    virtual int getClassId() = 0;

    virtual std::string toString() = 0;

    virtual Register* getResult() { return nullptr; }
};

class StoreInst : public Instruction {
public:
    StoreInst(Register* value, Register* dest, int imm, ByteLen len)
        : _value(value), _dest(dest), _imm(imm), _len(len) {}

    virtual std::string toString() override;

private:
    Register* _value;
    Register* _dest;
    int _imm;
    ByteLen _len;
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
    JumpInst(BasicBlock* targetBB) : _targetBB(targetBB) {}

    virtual std::string toString() override;

private:
    BasicBlock* _targetBB;
};

class CondJumpInst : public JumpInst {
public:
    CondJumpInst(int type, BasicBlock* targetBB);

    virtual std::string toString() override;
};

}  // namespace RISCV_ARCH
}  // namespace ATC
