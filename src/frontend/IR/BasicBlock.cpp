#include "IR/Function.h"

namespace ATC {
namespace IR {

BasicBlock::BasicBlock(Function* parent, const std::string& name) : _parent(parent), _name(name) {
    parent->insertBB(this);
    parent->insertName(this);
}

void BasicBlock::addInstruction(Instruction* inst) { _instructions.push_back(inst); }

std::string BasicBlock::getBBStr() { return _parent->getUniqueNameInFunction(this); }

}  // namespace IR
}  // namespace ATC