#pragma once

#include <iostream>

namespace ATC {

namespace AtomIR {
class Module;
class Function;
class BasicBlock;
class Instruction;
}  // namespace AtomIR

namespace RISCV_ARCH {
class CodeGenerator {
public:
    void dump(std::ostream& os);

    void emitModule(AtomIR::Module*);

    void emitFunction(AtomIR::Function*);

    void emitBasicBlock(AtomIR::BasicBlock*);

private:
};

}  // namespace RISCV_ARCH

}  // namespace ATC