#pragma once

#include <set>
#include <string>

#include "BasicBlock.h"
namespace ATC {

namespace RISCV {

class Function {
public:
    void insertFront(BasicBlock* bb) { _basicBlocks.push_front(bb); }

    void addBasicBlock(BasicBlock* bb) { _basicBlocks.push_back(bb); }

    const std::list<BasicBlock*>& getBasicBlocks() { return _basicBlocks; }

    static std::set<Register*> AllRegInFunction;

private:
    std::list<BasicBlock*> _basicBlocks;
};

}  // namespace RISCV
}  // namespace ATC