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

    void emitBitCastInst(AtomIR::BitCastInst *);

    void emitRetInst(AtomIR::ReturnInst *);

    void emitUnaryInst(AtomIR::UnaryInst *);

    void emitBinaryInst(AtomIR::BinaryInst *);

    void emitCondJumpInst(AtomIR::CondJumpInst *);

private:
    Register *emitIntBinaryInst(int instType, AtomIR::Value *operand1, AtomIR::Value *operand2);

    Register *emitFloatBinaryInst(int instType, AtomIR::Value *operand1, AtomIR::Value *operand2);

    Register *loadConstInt(int value);

    Register *loadConstFloat(float value);

    Register *loadConst(AtomIR::Constant *);

    Register *getRegFromValue(AtomIR::Value *);

    Register *processIfImmOutOfRange(Register *src, int &offset);

private:
    Function *_currentFunction;
    BasicBlock *_currentBasicBlock;
    BasicBlock *retBB;

    int _offset = 0;                                         // record current offset of sp
    std::unordered_map<AtomIR::Value *, int> _value2offset;  // offset of sp

    std::unordered_map<AtomIR::Value *, Register *> _value2reg;  // IR value to asm reg

    std::set<AtomIR::Value *> _paramInStack;  // params saved in stack

    std::unordered_map<AtomIR::BasicBlock *, BasicBlock *> _atomBB2asmBB;  // IR BasicBlock to asm BasicBlock

    std::unordered_map<float, std::string> _float2lable;  // float constant global lable

    std::stringstream _contend;

    int _maxPassParamsStackOffset = 0;  // pass the function params
};

}  // namespace RISCV

}  // namespace ATC