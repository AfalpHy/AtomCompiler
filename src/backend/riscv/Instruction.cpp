#include "riscv/Instruction.h"

#include <assert.h>

#include "riscv/BasicBlock.h"

namespace ATC {

namespace RISCV {
std::string LoadInst::toString() {
    std::string str;
    switch (_type) {
        case INST_LB:
            str.append("lb");
            break;
        case INST_LH:
            str.append("lh");
            break;
        case INST_LW:
            str.append("lw");
            break;
        case INST_LD:
            str.append("ld");
            break;
        default:
            assert(0 && "should't reach here");
            break;
    }

    str.append("\t").append(_dest->getName()).append(", ").append(std::to_string(_imm) + "(" + _src1->getName() + ")");
    return str;
}

std::string StoreInst::toString() {
    std::string str;
    switch (_type) {
        case INST_SB:
            str.append("sb");
            break;
        case INST_SH:
            str.append("sh");
            break;
        case INST_SW:
            str.append("sw");
            break;
        case INST_SD:
            str.append("sd");
            break;

        default:
            assert(0 && "should't reach here");
            break;
    }

    str.append("\t").append(_src1->getName()).append(", ").append(std::to_string(_imm) + "(" + _src2->getName() + ")");
    return str;
}

std::string JumpInst::toString() { return "j\t" + _targetBB; }

std::string CondJumpInst::toString() {
    switch (_type) {
        case INST_BEQ: {
            return "beq\t" + _src1->getName() + ", " + _src2->getName() + ", " + _targetBB;
        }
        default:
            break;
    }
    return "";
}

std::string BinaryInst::toString() {
    switch (_type) {
        case INST_ADDI: {
            return "addi\t" + _dest->getName() + ", " + _src1->getName() + ", " + std::to_string(_imm);
        }
        case INST_ADD: {
            return "add\t" + _dest->getName() + ", " + _src1->getName() + ", " + _src2->getName();
        }
        case INST_MUL: {
            return "mul\t" + _dest->getName() + ", " + _src1->getName() + ", " + _src2->getName();
        }
        default:
            break;
    }
    return "";
}

std::string ImmInst::toString() {
    switch (_type) {
        case INST_LI: {
            return "li\t" + _dest->getName() + ", " + std::to_string(_imm);
        }
        default:
            break;
    }
    return "";
}

}  // namespace RISCV

}  // namespace ATC