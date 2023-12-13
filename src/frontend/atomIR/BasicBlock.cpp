#include "atomIR/Function.h"

namespace ATC {
namespace AtomIR {

BasicBlock::BasicBlock(Function* parent, const std::string& name) : _parent(parent), _name(name) {
    parent->insertBB(this);
}

void BasicBlock::addInstruction(Instruction* inst) { _instructions.push_back(inst); }

std::string BasicBlock::getBBStr() { return _parent->getUniqueNameInFunction(this); }

}  // namespace AtomIR
}  // namespace ATC