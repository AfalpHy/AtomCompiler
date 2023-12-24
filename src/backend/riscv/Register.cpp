
#include "riscv/Function.h"

namespace ATC {

namespace RISCV {

Register::Register(Instruction* defined, bool b) {
    _defined = defined;
    _intReg = b;
    _name = "virtual_reg" + std::to_string(Index++);
    Function::AllRegInFunction.insert(this);
}

}  // namespace RISCV
}  // namespace ATC
