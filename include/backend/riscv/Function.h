#pragma once

#include <string>
#include <vector>

namespace ATC {

namespace RISCV_ARCH {

class BasicBlock;

class Function {
public:
    void addBasicBlock(BasicBlock* bb) { _basicBlocks.push_back(bb); }

    const std::vector<BasicBlock*>& getBasicBlocks() { return _basicBlocks; }

private:
    std::vector<BasicBlock*> _basicBlocks;
};

}  // namespace RISCV_ARCH
}  // namespace ATC