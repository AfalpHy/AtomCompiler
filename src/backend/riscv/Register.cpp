
#include "riscv/Function.h"

namespace ATC {

namespace RISCV {

Register::Register() {
    _name = "virtual_reg" + std::to_string(Index++);
    Function::AllRegInFunction.insert(this);
}

}  // namespace RISCV
}  // namespace ATC
