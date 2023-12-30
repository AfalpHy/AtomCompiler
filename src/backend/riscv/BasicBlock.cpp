#include <iostream>

#include "riscv/Function.h"
namespace ATC {

namespace RISCV {

BasicBlock::BasicBlock(Function *function, const std::string &name) {
    function->addBasicBlock(this);
    if (name.empty()) {
        _name = ".LBB" + std::to_string(Index++);
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