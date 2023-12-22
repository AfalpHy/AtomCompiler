#pragma once

#include <set>
#include <string>
#include <vector>

#include "BasicBlock.h"
namespace ATC {

namespace RISCV {

class Function {
public:
    void addBasicBlock(BasicBlock* bb) { _basicBlocks.push_back(bb); }

    const std::vector<BasicBlock*>& getBasicBlocks() { return _basicBlocks; }

    static std::set<Register*> AllRegInFunction;

private:
    std::vector<BasicBlock*> _basicBlocks;
};

}  // namespace RISCV
}  // namespace ATC