#include "riscv/BasicBlock.h"

#include <iostream>
namespace ATC {

namespace RISCV {

void BasicBlock::addInstruction(Instruction *inst) {
    if (auto jumpInst = dynamic_cast<JumpInst *>(inst)) {
        _successors.push_back(jumpInst->getTargetBB());
        jumpInst->getTargetBB()->addPredecessor(this);
    }
    _instructions.push_back(inst);
}
void BasicBlock::dump() { std::cout << toString(); }
}  // namespace RISCV
}  // namespace ATC