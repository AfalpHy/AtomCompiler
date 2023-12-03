#include "atomIR/Function.h"

namespace ATC {
namespace AtomIR {

void BasicBlock::addInstruction(Instruction* inst) {
    if (!_instructions.empty()) {
        _parent->setNeedUpdateName(true);
    }

    for (int i = _parent->getBBIndex(this) + 1; i < _parent->getBasicBlocks().size(); i++) {
        if (!_parent->getBasicBlocks()[i]->getInstructionList().empty()) {
            _parent->setNeedUpdateName(true);
            break;
        }
    }

    _instructions.push_back(inst);
}

std::string BasicBlock::getBBStr() { return _parent->getUniqueNameInFunction(this); }

}  // namespace AtomIR
}  // namespace ATC