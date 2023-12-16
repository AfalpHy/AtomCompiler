#pragma once

#include <iostream>
#include <sstream>
#include <unordered_map>

#include "atomIR/Module.h"

namespace ATC {

namespace RISCV_ARCH {

class Function;
class BasicBlock;
class Register;

class CodeGenerator {
public:
    CodeGenerator();

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
    Register *loadConstInt(int value, const std::string &name = "");

    Register *loadConstFloat(float value, const std::string &name = "");

    Register *loadGlobalAddr(AtomIR::GloabalVariable *global);

private:
    Function *_currentFunction;
    BasicBlock *_currentBasicBlock;

    int _offset = 0;                                         // record current offset of sp
    std::unordered_map<AtomIR::Value *, int> _value2offset;  // offset of sp

    std::unordered_map<AtomIR::Value *, Register *> _value2reg;  // IR value to backend reg

    std::stringstream _contend;

    // common regs
    Register *_sp = nullptr;
};

}  // namespace RISCV_ARCH

}  // namespace ATC