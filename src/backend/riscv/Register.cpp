
#include "riscv/Function.h"

namespace ATC {

namespace RISCV {

Register::Register(bool b) {
    _intReg = b;
    _name = "virtual_reg" + std::to_string(Index++);
}

void Register::reset() {
    _name = "virtual_reg" + std::to_string(Index++);
    _interferences.clear();
}

}  // namespace RISCV
}  // namespace ATC
