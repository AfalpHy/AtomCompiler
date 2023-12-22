#include <assert.h>

#include "riscv/BasicBlock.h"

namespace ATC {

namespace RISCV {

std::string ImmInst::toString() {
    switch (_type) {
        case INST_LI:
            return "li\t" + _dest->getName() + ", " + std::to_string(_imm);
        default:
            assert(0 && "unsupported");
            break;
    }
}

std::string LoadInst::toString() {
    switch (_type) {
        case INST_LB:
            return "lb\t" + _dest->getName() + ", " + std::to_string(_imm) + "(" + _src1->getName() + ")";
        case INST_LH:
            return "lh\t" + _dest->getName() + ", " + std::to_string(_imm) + "(" + _src1->getName() + ")";
        case INST_LW:
            return "lw\t" + _dest->getName() + ", " + std::to_string(_imm) + "(" + _src1->getName() + ")";
        case INST_LD:
            return "ld\t" + _dest->getName() + ", " + std::to_string(_imm) + "(" + _src1->getName() + ")";
        case INST_FLW:
            return "flw\t" + _dest->getName() + ", " + std::to_string(_imm) + "(" + _src1->getName() + ")";
        case INST_FLD:
            return "fld\t" + _dest->getName() + ", " + std::to_string(_imm) + "(" + _src1->getName() + ")";
        default:
            assert(0 && "unsupported");
            break;
    }
}

std::string StoreInst::toString() {
    switch (_type) {
        case INST_SB:
            return "sb\t" + _src1->getName() + ", " + std::to_string(_imm) + "(" + _src2->getName() + ")";
        case INST_SH:
            return "sh\t" + _src1->getName() + ", " + std::to_string(_imm) + "(" + _src2->getName() + ")";
        case INST_SW:
            return "sw\t" + _src1->getName() + ", " + std::to_string(_imm) + "(" + _src2->getName() + ")";
        case INST_SD:
            return "sd\t" + _src1->getName() + ", " + std::to_string(_imm) + "(" + _src2->getName() + ")";
        case INST_FSW:
            return "fsw\t" + _src1->getName() + ", " + std::to_string(_imm) + "(" + _src2->getName() + ")";
        case INST_FSD:
            return "fsd\t" + _src1->getName() + ", " + std::to_string(_imm) + "(" + _src2->getName() + ")";
        default:
            assert(0 && "should't reach here");
            break;
    }
}

std::string UnaryInst::toString() {
    switch (_type) {
        case INST_FCVT_S_W:
            return "fcvt.s.w\t" + _dest->getName() + ", " + _src1->getName();
        case INST_FCVT_W_S:
            return "fcvt.w.s\t" + _dest->getName() + ", " + _src1->getName();
        case INST_SEQZ:
            return "seqz\t" + _dest->getName() + ", " + _src1->getName();
        case INST_SNEZ:
            return "snez\t" + _dest->getName() + ", " + _src1->getName();
        case INST_FMV_W_X:
            return "fmv.w.x\t" + _dest->getName() + ", " + _src1->getName();
        default:
            assert(0 && "unsupported");
            break;
    }
}

std::string BinaryInst::toString() {
    switch (_type) {
        case INST_ADDI:
            return "addi\t" + _dest->getName() + ", " + _src1->getName() + ", " + std::to_string(_imm);
        case INST_SLTI:
            return "slti\t" + _dest->getName() + ", " + _src1->getName() + ", " + std::to_string(_imm);
        case INST_XORI:
            return "xori\t" + _dest->getName() + ", " + _src1->getName() + ", " + std::to_string(_imm);
        case INST_ADD:
            return "add\t" + _dest->getName() + ", " + _src1->getName() + ", " + _src2->getName();
        case INST_SUB:
            return "sub\t" + _dest->getName() + ", " + _src1->getName() + ", " + _src2->getName();
        case INST_SLT:
            return "slt\t" + _dest->getName() + ", " + _src1->getName() + ", " + _src2->getName();
        case INST_XOR:
            return "xor\t" + _dest->getName() + ", " + _src1->getName() + ", " + _src2->getName();
        case INST_MUL:
            return "mul\t" + _dest->getName() + ", " + _src1->getName() + ", " + _src2->getName();
        case INST_DIV:
            return "div\t" + _dest->getName() + ", " + _src1->getName() + ", " + _src2->getName();
        case INST_REM:
            return "rem\t" + _dest->getName() + ", " + _src1->getName() + ", " + _src2->getName();
        case INST_FADD_S:
            return "fadd.s\t" + _dest->getName() + ", " + _src1->getName() + ", " + _src2->getName();
        case INST_FSUB_S:
            return "fsub.s\t" + _dest->getName() + ", " + _src1->getName() + ", " + _src2->getName();
        case INST_FMUL_S:
            return "fmul.s\t" + _dest->getName() + ", " + _src1->getName() + ", " + _src2->getName();
        case INST_FDIV_S:
            return "fdiv.s\t" + _dest->getName() + ", " + _src1->getName() + ", " + _src2->getName();
        case INST_FSLT_S:
            return "flt.s\t" + _dest->getName() + ", " + _src1->getName() + ", " + _src2->getName();
        case INST_FSLE_S:
            return "fle.s\t" + _dest->getName() + ", " + _src1->getName() + ", " + _src2->getName();
        case INST_FSEQ_S:
            return "feq.s\t" + _dest->getName() + ", " + _src1->getName() + ", " + _src2->getName();
        default:
            assert(0 && "unsupported");
            break;
    }
}

std::string JumpInst::toString() { return "j\t" + _targetBB->getName(); }

std::string CondJumpInst::toString() {
    switch (_type) {
        case INST_BEQ:
            return "beq\t" + _src1->getName() + ", " + _src2->getName() + ", " + _targetBB->getName();
        case INST_BNE:
            return "bne\t" + _src1->getName() + ", " + _src2->getName() + ", " + _targetBB->getName();
        case INST_BLT:
            return "blt\t" + _src1->getName() + ", " + _src2->getName() + ", " + _targetBB->getName();
        case INST_BGE:
            return "bge\t" + _src1->getName() + ", " + _src2->getName() + ", " + _targetBB->getName();
        default:
            assert(0 && "unsupported");
            break;
    }
}

}  // namespace RISCV

}  // namespace ATC