#include "riscv/Instruction.h"

#include <assert.h>

#include "riscv/BasicBlock.h"

namespace ATC {

namespace RISCV_ARCH {
static std::string getStringFromByteLen(ByteLen len) {
    switch (len) {
        case BYTE:
            return "b";
        case HALF_WORD:
            return "h";
        case WORD:
            return "w";
        case DOUBLE_WORD:
            return "d";
        default:
            assert(false && "should not reach herer");
            break;
    }
}

std::string StoreInst::toString() {
    std::string str = "s";
    str.append(getStringFromByteLen(_len))
        .append("\t")
        .append(_value->getName())
        .append(", ")
        .append(std::to_string(_imm) + "(" + _dest->getName() + ")");
    return str;
}

std::string JumpInst::toString() { return "j\t" + _targetBB->getName(); }

}  // namespace RISCV_ARCH

}  // namespace ATC