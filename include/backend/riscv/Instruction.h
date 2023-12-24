#pragma once

#include "Register.h"

namespace ATC {
namespace RISCV {

class BasicBlock;

enum InstId {
    ID_IMM_INST,
    ID_LOAD_INST,
    ID_STORE_INST,
    ID_FUNCTION_CALL_INST,
    ID_RETURN_INST,
    ID_UNARY_INST,
    ID_BINARY_INST,
    ID_LOAD_GLOBAL_ADDR_INST,
    ID_JUMP_INST,
    ID_COND_JUMP_INST
};

class Instruction {
public:
    virtual int getClassId() = 0;

    virtual std::string toString() = 0;

    int getInstType() { return _type; }

    Register* getDest() { return _dest; }

    Register* getSrc1() { return _src1; }

    Register* getSrc2() { return _src2; }

    int getImm() { return _imm; }

protected:
    int _type = -1;
    Register* _dest = nullptr;
    Register* _src1 = nullptr;
    Register* _src2 = nullptr;
    int _imm = 0;
};

class ImmInst : public Instruction {
public:
    ImmInst(int type, int imm) {
        _type = type;
        _imm = imm;
        _dest = new Register();
    }

    virtual int getClassId() override { return ID_IMM_INST; }

    virtual std::string toString() override;

    enum { INST_LI, INST_LUI, INST_AUIPC };
};

class LoadGlobalAddrInst : public Instruction {
public:
    LoadGlobalAddrInst(const std::string& name) : _name(name) { _dest = new Register(); }

    virtual int getClassId() override { return ID_LOAD_GLOBAL_ADDR_INST; }

    virtual std::string toString() override { return "la\t" + _dest->getName() + ", " + _name; }

private:
    std::string _name;
};

class LoadInst : public Instruction {
public:
    LoadInst(int type, Register* src1, int imm) {
        _type = type;
        _src1 = src1;
        _imm = imm;
        switch (type) {
            case INST_FLW:
            case INST_FLD:
                _dest = new Register(false);
                break;
            default:
                _dest = new Register();
                break;
        }
    }

    LoadInst(int type, Register* dest, Register* src1, int imm) {
        _type = type;
        _dest = dest;
        _src1 = src1;
        _imm = imm;
    }

    virtual int getClassId() override { return ID_LOAD_INST; }

    virtual std::string toString() override;

    enum { INST_LB, INST_LH, INST_LW, INST_LD, INST_LBU, INST_LHU, INST_LWU, INST_FLW, INST_FLD };
};

class StoreInst : public Instruction {
public:
    StoreInst(int type, Register* src1, Register* src2, int imm) {
        _type = type;
        _src1 = src1;
        _src2 = src2;
        _imm = imm;
    }

    virtual int getClassId() override { return ID_STORE_INST; }

    virtual std::string toString() override;

    enum { INST_SB, INST_SH, INST_SW, INST_SD, INST_FSW, INST_FSD };
};

class FunctionCallInst : public Instruction {
public:
    FunctionCallInst(const std::string& funcName) : _funcName(funcName) {}

    virtual int getClassId() override { return ID_FUNCTION_CALL_INST; }

    virtual std::string toString() override { return "call\t" + _funcName; }

private:
    std::string _funcName;
};

class ReturnInst : public Instruction {
public:
    virtual int getClassId() override { return ID_RETURN_INST; }

    virtual std::string toString() override { return "ret"; }
};

class UnaryInst : public Instruction {
public:
    UnaryInst(int type, Register* src1) {
        _type = type;
        _src1 = src1;
        switch (type) {
            case INST_FMV_S:
            case INST_FCVT_S_W:
            case INST_FMV_W_X:
                _dest = new Register(false);
                break;
            default:
                _dest = new Register();
                break;
        }
    }

    UnaryInst(int type, Register* dest, Register* src1) {
        _type = type;
        _src1 = src1;
        _dest = dest;
    }

    virtual int getClassId() override { return ID_UNARY_INST; }

    virtual std::string toString() override;

    enum { INST_MV, INST_FMV_S, INST_FCVT_S_W, INST_FCVT_W_S, INST_SEQZ, INST_SNEZ, INST_FMV_W_X };
};

class BinaryInst : public Instruction {
public:
    BinaryInst(int type, Register* src1, Register* src2) {
        _type = type;
        _src1 = src1;
        _src2 = src2;
        switch (type) {
            case INST_FADD_S:
            case INST_FSUB_S:
            case INST_FMUL_S:
            case INST_FDIV_S:
                _dest = new Register(false);
                break;
            default:
                _dest = new Register();
                break;
        }
    }

    BinaryInst(int type, Register* src1, int imm) {
        _type = type;
        _src1 = src1;
        _imm = imm;
        _dest = new Register();
    }

    BinaryInst(int type, Register* dest, Register* src1, int imm) {
        _type = type;
        _dest = dest;
        _src1 = src1;
        _imm = imm;
    }

    virtual int getClassId() override { return ID_BINARY_INST; }

    virtual std::string toString() override;

    enum {
        INST_ADDI,
        INST_SLTI,
        INST_XORI,

        INST_ADDIW,

        INST_ADD,
        INST_SUB,
        INST_SLT,
        INST_XOR,
        INST_MUL,
        INST_DIV,
        INST_REM,

        INST_ADDW,
        INST_SUBW,
        INST_MULW,
        INST_DIVW,
        INST_REMW,

        INST_FADD_S,
        INST_FSUB_S,
        INST_FMUL_S,
        INST_FDIV_S,
        INST_FSLT_S,
        INST_FSLE_S,
        INST_FSEQ_S,
        INST_FSNE_S // just for next inst, the asm code same as INST_FSEQ_S 
    };
};

class JumpInst : public Instruction {
public:
    JumpInst(BasicBlock* targetBB) : _targetBB(targetBB) {}

    BasicBlock* getTargetBB() { return _targetBB; }

    virtual int getClassId() override { return ID_JUMP_INST; }

    virtual std::string toString() override;

protected:
    BasicBlock* _targetBB;
};

class CondJumpInst : public JumpInst {
public:
    CondJumpInst(int type, Register* src1, Register* src2, BasicBlock* targetBB) : JumpInst(targetBB) {
        _type = type;
        _src1 = src1;
        _src2 = src2;
    }

    virtual int getClassId() override { return ID_COND_JUMP_INST; }

    virtual std::string toString() override;

    enum { INST_BEQ, INST_BNE, INST_BLT, INST_BGE };
};

}  // namespace RISCV
}  // namespace ATC
