
#include "riscv/Function.h"

namespace ATC {

namespace RISCV {

Register::Register(bool b) {
    _intReg = b;
    _name = "virtual_reg" + std::to_string(Index++);
    Function::AllRegInFunction.insert(this);
}

}  // namespace RISCV
}  // namespace ATC
