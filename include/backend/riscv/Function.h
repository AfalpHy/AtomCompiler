#pragma once

#include <vector>

namespace ATC {

namespace RISCV_ARCH {

class BasicBlock;
class Module;

class Function {
public:
private:
    std::vector<Function*> _functions;
};

}  // namespace RISCV_ARCH
}  // namespace ATC