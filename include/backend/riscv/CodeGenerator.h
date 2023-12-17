#pragma once

#include <iostream>
#include <sstream>
#include <unordered_map>

#include "atomIR/Module.h"

namespace ATC {

namespace RISCV {

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
    Register *loadConstInt(int value);

    Register *loadConstFloat(float value);

    Register *loadConst(AtomIR::Constant *);

    Register *getRegFromValue(AtomIR::Value *);

private:
    Function *_currentFunction;
    BasicBlock *_currentBasicBlock;

    int _offset = 0;                                         // record current offset of sp
    std::unordered_map<AtomIR::Value *, int> _value2offset;  // offset of sp

    std::unordered_map<AtomIR::Value *, Register *> _value2reg;  // IR value to asm reg

    std::unordered_map<AtomIR::BasicBlock *, std::string> _bb2lable;  // IR BasicBlock to asm lable

    std::stringstream _contend;

    // common regs
    Register *_sp = nullptr;
};

}  // namespace RISCV

}  // namespace ATC