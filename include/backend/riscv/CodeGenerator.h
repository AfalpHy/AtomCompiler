#pragma once

#include <iostream>

namespace ATC {

namespace AtomIR {
class Module;
class Function;
class BasicBlock;
class Instruction;
class GloabalVariable;
}  // namespace AtomIR

namespace RISCV_ARCH {

class Module;
class Function;
class BasicBlock;

class CodeGenerator {
public:
    void dump(std::ostream &os);

    void emitModule(AtomIR::Module *);

    void emitGlobalVariable(AtomIR::GloabalVariable *);

    void emitFunction(AtomIR::Function *);

    void emitBasicBlock(AtomIR::BasicBlock *);

    void emitInstruction(AtomIR::Instruction *);

private:
    Module *_currentModule;
    Function *_currentFunction;
    BasicBlock *_currentBasicBlock;
};

}  // namespace RISCV_ARCH

}  // namespace ATC