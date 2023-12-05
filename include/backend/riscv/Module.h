#pragma once

#include <vector>

namespace ATC {

namespace RISCV_ARCH {

class Function;

class Module {
public:
private:
    std::vector<Function*> _functions;
};

}  // namespace RISCV_ARCH
}  // namespace ATC