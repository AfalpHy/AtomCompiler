#include "riscv/BasicBlock.h"

#include <iostream>
namespace ATC {

namespace RISCV {

BasicBlock::BasicBlock(const std::string &name) {
    if (name.empty()) {
        _name = ".L" + std::to_string(Index++);
    } else {
        _name = name;
    }
}

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