#pragma once

#include <iostream>
#include <sstream>
#include <unordered_map>

#include "atomIR/Module.h"

namespace ATC {

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

    void emitAllocInst(AtomIR::AllocInst *);

    void emitStoreInst(AtomIR::StoreInst *);

    void emitFunctionCallInst(AtomIR::FunctionCallInst *);

    void emitGEPInst(AtomIR::GetElementPtrInst *);

    void emitRetInst(AtomIR::ReturnInst *);

    void emitUnaryInst(AtomIR::UnaryInst *);

    void emitBinaryInst(AtomIR::BinaryInst *);

    void emitCondJumpInst(AtomIR::CondJumpInst *);

private:
    Function *_currentFunction;
    BasicBlock *_currentBasicBlock;

    std::unordered_map<AtomIR::Value *, int> _value2offset;  // offset of sp

    std::stringstream _contend;
};

}  // namespace RISCV_ARCH

}  // namespace ATC