#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include "IR/Module.h"

namespace ATC {

namespace RISCV {

class Function;
class BasicBlock;
class Register;

class CodeGenerator {
public:
    CodeGenerator();

    void dump();

    void print(std::ofstream &os);

    void emitModule(IR::Module *);

    void emitGlobalVariable(IR::GloabalVariable *);

    void emitFunction(IR::Function *);

    void emitBasicBlock(IR::BasicBlock *);

    void emitInstruction(IR::Instruction *);

    void emitAllocInst(IR::AllocInst *);

    void emitStoreInst(IR::StoreInst *);

    void emitFunctionCallInst(IR::FunctionCallInst *);

    void emitGEPInst(IR::GetElementPtrInst *);

    void emitBitCastInst(IR::BitCastInst *);

    void emitRetInst(IR::ReturnInst *);

    void emitUnaryInst(IR::UnaryInst *);

    void emitBinaryInst(IR::BinaryInst *);

    void emitCondJumpInst(IR::CondJumpInst *);

private:
    Register *emitIntBinaryInst(int instType, IR::Value *operand1, IR::Value *operand2);

    Register *emitFloatBinaryInst(int instType, IR::Value *operand1, IR::Value *operand2);

    Register *loadConstInt(int value);

    Register *loadConstFloat(float value);

    Register *loadConst(IR::Constant *);

    Register *getRegFromValue(IR::Value *);

    Register *processIfImmOutOfRange(Register *src, int &offset);

private:
    Function *_currentFunction;
    BasicBlock *_currentBasicBlock;
    BasicBlock *_entryBB;
    BasicBlock *_retBB;

    int _offset = 0;                                         // record current offset of sp
    std::unordered_map<IR::Value *, int> _value2offset;  // offset of sp

    std::unordered_map<IR::Value *, Register *> _value2reg;  // IR value to asm reg

    std::set<IR::Value *> _paramInStack;  // params saved in stack

    std::unordered_map<IR::BasicBlock *, BasicBlock *> _IRBB2asmBB;  // IR BasicBlock to asm BasicBlock

    std::unordered_map<float, std::string> _float2lable;  // float constant global lable

    std::stringstream _contend;

    int _maxPassParamsStackOffset = 0;  // pass the function params
};

}  // namespace RISCV

}  // namespace ATC