#pragma once

#include <iostream>
#include <sstream>
namespace ATC {

namespace AtomIR {
class Module;
class Function;
class BasicBlock;
class Instruction;
class GloabalVariable;
}  // namespace AtomIR

namespace RISCV_ARCH {

class Function;
class BasicBlock;

class CodeGenerator {
public:
    void dump(std::ostream &os = std::cout);

    void emitModule(AtomIR::Module *);

    void emitGlobalVariable(AtomIR::GloabalVariable *);

    void emitFunction(AtomIR::Function *);

    void emitBasicBlock(AtomIR::BasicBlock *);

    void emitInstruction(AtomIR::Instruction *);

private:
    Function *_currentFunction;
    BasicBlock *_currentBasicBlock;
    std::stringstream _contend;
};

}  // namespace RISCV_ARCH

}  // namespace ATC