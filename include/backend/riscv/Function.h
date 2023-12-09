#pragma once

#include <string>
#include <vector>

namespace ATC {

namespace RISCV_ARCH {

class BasicBlock;

class Function {
public:

private:
    std::vector<BasicBlock*> _basicBlocks;
};

}  // namespace RISCV_ARCH
}  // namespace ATC