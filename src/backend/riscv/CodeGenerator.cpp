#include "riscv/CodeGenerator.h"

#include <assert.h>

#include "atomIR/Instruction.h"
#include "atomIR/Module.h"
#include "riscv/BasicBlock.h"
#include "riscv/Function.h"
#include "riscv/RegAllocator.h"

namespace ATC {
namespace RISCV {

using std::endl;

int BasicBlock::Index = 0;
int Register::Index = 0;

std::set<Register*> Function::AllRegInFunction;
std::vector<Register*> Function::CallerSavedRegs;

// common regs
Register* Register::Ra = nullptr;
Register* Register::S0 = nullptr;
Register* Register::Sp = nullptr;
Register* Register::Zero = nullptr;

std::vector<Register*> Register::IntArgReg;
std::vector<Register*> Register::FloatArgReg;

CodeGenerator::CodeGenerator() {
    Register::Ra = new Register();
    Register::Ra->setName("ra");
    Register::Ra->setIsFixed(true);

    Register::S0 = new Register();
    Register::S0->setName("s0");
    Register::S0->setIsFixed(true);

    Register::Sp = new Register();
    Register::Sp->setName("sp");
    Register::Sp->setIsFixed(true);

    Register::Zero = new Register();
    Register::Zero->setName("zero");
    Register::Zero->setIsFixed(true);

    for (int i = 0; i < 8; i++) {
        auto argReg = new Register();
        argReg->setName("a" + std::to_string(i));
        argReg->setIsFixed(true);
        Register::IntArgReg.push_back(argReg);
        Function::CallerSavedRegs.push_back(argReg);

        argReg = new Register(nullptr, false);
        argReg->setName("fa" + std::to_string(i));
        argReg->setIsFixed(true);
        Register::FloatArgReg.push_back(argReg);
        Function::CallerSavedRegs.push_back(argReg);
    }
    /// TODO:add t0-t6 to Function::CallerSavedRegs
}

void CodeGenerator::dump(std::ostream& os) { os << _contend.str() << endl; }

void CodeGenerator::emitModule(AtomIR::Module* module) {
    if (!module->getGlobalVariables().empty()) {
        _contend << "\t.data" << endl;
    }
    for (auto& item : module->getGlobalVariables()) {
        emitGlobalVariable(item);
    }

    if (!module->getFunctions().empty()) {
        _contend << "\t.text" << endl;
    }
    for (auto& item : module->getFunctions()) {
        emitFunction(item);
    }

    if (!_float2lable.empty()) {
        _contend << "\t.section\t.rdata" << endl;
    }
    for (auto& [value, lable] : _float2lable) {
        _contend << "\t.p2align\t2" << endl;
        _contend << lable << ":" << endl;
        _contend << "\t.word\t" << std::to_string(*(int*)(&value)) << endl;
    }
}

void CodeGenerator::emitGlobalVariable(AtomIR::GloabalVariable* var) {
    _contend << "\t.type\t" << var->getName() << ",@object" << endl;
    _contend << "\t.globl\t" << var->getName() << endl;
    _contend << "\t.p2align\t2" << endl;
    _contend << var->getName() << ":" << endl;
    int size = 0;
    if (auto arrayValue = dynamic_cast<AtomIR::ArrayValue*>(var->getInitialValue())) {
        for (auto& item : arrayValue->getElements()) {
            if (item.second.empty()) {
                size += item.first * 4;
                _contend << "\t.zero\t" << std::to_string(item.first * 4) << endl;
            } else {
                for (auto& element : item.second) {
                    size += 4;
                    _contend << "\t.word\t" << static_cast<AtomIR::Constant*>(element)->getLiteralStr() << endl;
                }
            }
        }
    } else {
        size = 4;
        _contend << "\t.word\t" << static_cast<AtomIR::Constant*>(var->getInitialValue())->getLiteralStr() << endl;
    }
    _contend << "\t.size\t" << var->getName() << ", " << std::to_string(size) << endl << endl;
}

void CodeGenerator::emitFunction(AtomIR::Function* function) {
    // reset
    if (function->hasFunctionCall()) {
        // save ra and s0
        _offset = -16;
    } else {
        // save s0
        _offset = -8;
    }
    _value2offset.clear();
    Function::AllRegInFunction.clear();

    _currentFunction = new Function();

    int intOrder = 0;
    int floatOrder = 0;
    for (auto param : function->getParams()) {
        if (param->getType()->getTypeEnum() == AtomIR::INT32_TY) {
            _value2reg[param] = Register::IntArgReg[intOrder++];
        } else {
            _value2reg[param] = Register::FloatArgReg[floatOrder++];
        }
    }
    auto entryBB = new BasicBlock("");
    retBB = new BasicBlock("." + function->getName() + "_ret");
    _currentFunction->addBasicBlock(entryBB);

    // prepare the BasicBlock
    for (auto bb : function->getBasicBlocks()) {
        _atomBB2asmBB[bb] = new BasicBlock();
    }

    for (auto bb : function->getBasicBlocks()) {
        emitBasicBlock(bb);
    }
    _currentFunction->addBasicBlock(retBB);

    RegAllocator regAllocator(_currentFunction, _offset, true);
    regAllocator.run();

    // The stack is aligned to 16 bytes
    if (_offset % 16 != 0) {
        _offset = (_offset - 15) / 16 * 16;
    }
    entryBB->addInstruction(new BinaryInst(BinaryInst::INST_ADDI, Register::Sp, Register::Sp, _offset));
    if (function->hasFunctionCall()) {
        entryBB->addInstruction(new StoreInst(StoreInst::INST_SD, Register::Ra, Register::Sp, -_offset - 8));
        entryBB->addInstruction(new StoreInst(StoreInst::INST_SD, Register::S0, Register::Sp, -_offset - 16));
        retBB->addInstruction(new LoadInst(LoadInst::INST_LD, Register::Ra, Register::Sp, -_offset - 8));
        retBB->addInstruction(new LoadInst(LoadInst::INST_LD, Register::S0, Register::Sp, -_offset - 16));
    } else {
        entryBB->addInstruction(new StoreInst(StoreInst::INST_SD, Register::S0, Register::Sp, -_offset - 8));
        retBB->addInstruction(new LoadInst(LoadInst::INST_LD, Register::S0, Register::Sp, -_offset - 8));
    }
    entryBB->addInstruction(new BinaryInst(BinaryInst::INST_ADDI, Register::S0, Register::Sp, -_offset));
    retBB->addInstruction(new BinaryInst(BinaryInst::INST_ADDI, Register::Sp, Register::Sp, -_offset));
    retBB->addInstruction(new ReturnInst());

    _contend << "\t.globl\t" << function->getName() << endl;
    _contend << "\t.p2align\t1" << endl;
    _contend << "\t.type\t" << function->getName() << ",@function" << endl;
    _contend << function->getName() << ":" << endl;
    // append completed code into content
    for (auto bb : _currentFunction->getBasicBlocks()) {
        _contend << bb->toString();
    }
    _contend << "\t.size\t" << function->getName() << ", .-" << function->getName() << endl << endl;
}

void CodeGenerator::emitBasicBlock(AtomIR::BasicBlock* basicBlock) {
    _currentBasicBlock = _atomBB2asmBB[basicBlock];
    _currentFunction->addBasicBlock(_currentBasicBlock);
    for (auto inst : basicBlock->getInstructionList()) {
        emitInstruction(inst);
    }
}

void CodeGenerator::emitInstruction(AtomIR::Instruction* inst) {
    switch (inst->getClassId()) {
        case AtomIR::ID_ALLOC_INST:
            emitAllocInst((AtomIR::AllocInst*)inst);
            break;
        case AtomIR::ID_STORE_INST:
            emitStoreInst((AtomIR::StoreInst*)inst);
            break;
        case AtomIR::ID_FUNCTION_CALL_INST:
            emitFunctionCallInst((AtomIR::FunctionCallInst*)inst);
            break;
        case AtomIR::ID_GETELEMENTPTR_INST:
            emitGEPInst((AtomIR::GetElementPtrInst*)inst);
            break;
        case AtomIR::ID_BITCAST_INST:
            emitBitCastInst((AtomIR::BitCastInst*)inst);
            break;
        case AtomIR::ID_RETURN_INST:
            emitRetInst((AtomIR::ReturnInst*)inst);
            break;
        case AtomIR::ID_UNARY_INST:
            emitUnaryInst((AtomIR::UnaryInst*)inst);
            break;
        case AtomIR::ID_BINARY_INST:
            emitBinaryInst((AtomIR::BinaryInst*)inst);
            break;
        case AtomIR::ID_JUMP_INST: {
            AtomIR::JumpInst* jumpInst = (AtomIR::JumpInst*)inst;
            _currentBasicBlock->addInstruction(new JumpInst(_atomBB2asmBB[jumpInst->getTargetBB()]));
            break;
        }
        case AtomIR::ID_COND_JUMP_INST:
            emitCondJumpInst((AtomIR::CondJumpInst*)inst);
            break;
        default:
            assert(false && "should not reach here");
            break;
    }
}

void CodeGenerator::emitAllocInst(AtomIR::AllocInst* inst) {
    auto type = inst->getResult()->getType()->getBaseType();
    _offset -= type->getByteLen();
    _value2offset[inst->getResult()] = _offset;
    _value2reg[inst->getResult()] = Register::S0;
}

void CodeGenerator::emitStoreInst(AtomIR::StoreInst* inst) {
    int instType;
    if (inst->isIntInst()) {
        instType = StoreInst::INST_SW;
    } else {
        instType = StoreInst::INST_FSW;
    }

    auto value = inst->getValue();
    auto dest = inst->getDest();
    Register* src1 = getRegFromValue(value);
    Register* src2 = getRegFromValue(dest);
    int offset = 0;
    if (!dest->isGlobal()) {
        offset = _value2offset[dest];
    }

    _currentBasicBlock->addInstruction(new StoreInst(instType, src1, src2, offset));
}

void CodeGenerator::emitFunctionCallInst(AtomIR::FunctionCallInst* inst) {
    int intOrder = 0;
    int floatOrder = 0;
    for (auto param : inst->getParams()) {
        auto paramReg = getRegFromValue(param);
        if (param->getType()->isPointerType() || param->getType()->getTypeEnum() == AtomIR::INT32_TY) {
            if (intOrder < 8) {
                _currentBasicBlock->addInstruction(
                    new UnaryInst(UnaryInst::INST_MV, Register::IntArgReg[intOrder++], paramReg));
            } else {
            }
        } else {
            if (floatOrder < 8) {
                _currentBasicBlock->addInstruction(
                    new UnaryInst(UnaryInst::INST_FMV_S, Register::FloatArgReg[floatOrder++], paramReg));
            } else {
            }
        }
    }
    auto call = new FunctionCallInst(inst->getFuncName());
    for (int i = 0; i < intOrder; i++) {
        call->addUsedReg(Register::IntArgReg[i]);
    }
    for (int i = 0; i < floatOrder; i++) {
        call->addUsedReg(Register::FloatArgReg[i]);
    }
    _currentBasicBlock->addInstruction(call);
    if (inst->getResult()) {
        Instruction* mv;
        if (inst->getResult()->getType()->getTypeEnum() == AtomIR::INT32_TY) {
            mv = new UnaryInst(UnaryInst::INST_MV, Register::IntArgReg[0]);
        } else {
            mv = new UnaryInst(UnaryInst::INST_FMV_S, Register::FloatArgReg[0]);
        }
        _currentBasicBlock->addInstruction(mv);
        _value2reg[inst->getResult()] = mv->getDest();
    }
}

void CodeGenerator::emitGEPInst(AtomIR::GetElementPtrInst* inst) {
    Register* ptr = getRegFromValue(inst->getPtr());
    auto& indexes = inst->getIndexes();
    if (indexes.size() == 1) {
        int offset = inst->getPtr()->getType()->getBaseType()->getByteLen();
        _value2reg[inst->getResult()] = ptr;
        _value2offset[inst->getResult()] = offset;
    } else {
        if (indexes[1]->isConst()) {
            _value2reg[inst->getResult()] = ptr;
            _value2offset[inst->getResult()] = static_cast<AtomIR::ConstantInt*>(indexes[1])->getConstValue() * 4;
        } else {
            auto li = new ImmInst(ImmInst::INST_LI, 4);
            _currentBasicBlock->addInstruction(li);
            auto mul = new BinaryInst(BinaryInst::INST_MUL, _value2reg[indexes[1]], li->getDest());
            _currentBasicBlock->addInstruction(mul);
            auto add = new BinaryInst(BinaryInst::INST_ADD, ptr, mul->getDest());
            _currentBasicBlock->addInstruction(add);
            _value2reg[inst->getResult()] = add->getDest();
        }
    }
}

void CodeGenerator::emitBitCastInst(AtomIR::BitCastInst* inst) {
    Register* ptr = getRegFromValue(inst->getPtr());
    if (ptr == Register::S0) {
        auto getPtr = new BinaryInst(BinaryInst::INST_ADDI, Register::S0, _value2offset[inst->getPtr()]);
        _value2reg[inst->getResult()] = getPtr->getDest();
        _currentBasicBlock->addInstruction(getPtr);
    } else {
        _value2reg[inst->getResult()] = ptr;
    }
}

void CodeGenerator::emitRetInst(AtomIR::ReturnInst* inst) {
    if (inst->getResult()) {
        if (inst->getResult()->getType()->getTypeEnum() == AtomIR::INT32_TY) {
            _currentBasicBlock->addInstruction(
                new UnaryInst(UnaryInst::INST_MV, Register::IntArgReg[0], _value2reg[inst->getResult()]));
        } else {
            _currentBasicBlock->addInstruction(
                new UnaryInst(UnaryInst::INST_FMV_S, Register::FloatArgReg[0], _value2reg[inst->getResult()]));
        }
    }
    _currentBasicBlock->addInstruction(new JumpInst(retBB));
}

void CodeGenerator::emitUnaryInst(AtomIR::UnaryInst* inst) {
    Register* src1 = getRegFromValue(inst->getOperand());
    Instruction* unaryInst = nullptr;
    switch (inst->getInstType()) {
        case AtomIR::UnaryInst::INST_LOAD: {
            int offset = inst->getOperand()->isGlobal() ? 0 : _value2offset[inst->getOperand()];
            unaryInst = new LoadInst(
                inst->getResult()->getType() == AtomIR::Type::getInt32Ty() ? LoadInst::INST_LW : LoadInst::INST_FLW,
                src1, offset);
            break;
        }
        case AtomIR::UnaryInst::INST_ITOF: {
            if (src1->getDefined() && src1->getDefined()->getClassId() == ID_BINARY_INST &&
                src1->getDefined()->getInstType() == BinaryInst::INST_FSEQ_S) {
                auto zeroBB = new BasicBlock();
                _currentFunction->addBasicBlock(zeroBB);
                auto afterBB = new BasicBlock();
                _currentFunction->addBasicBlock(afterBB);
                auto beq = new CondJumpInst(CondJumpInst::INST_BEQ, src1, Register::Zero, zeroBB);
                _currentBasicBlock->addInstruction(beq);
                Register* dest = loadConstFloat(1);
                _currentBasicBlock->addInstruction(new JumpInst(afterBB));
                zeroBB->addInstruction(new UnaryInst(UnaryInst::INST_FMV_W_X, dest, Register::Zero));
                _currentBasicBlock = afterBB;
                _value2reg[inst->getResult()] = dest;
                return;
            }
            unaryInst = new UnaryInst(UnaryInst::INST_FCVT_S_W, src1);
            break;
        }
        case AtomIR::UnaryInst::INST_FTOI:
            unaryInst = new UnaryInst(UnaryInst::INST_FCVT_W_S, src1);
            break;
        default:
            assert(0 && "shouldn't reach here");
            break;
    }
    _currentBasicBlock->addInstruction(unaryInst);
    _value2reg[inst->getResult()] = unaryInst->getDest();
}

void CodeGenerator::emitBinaryInst(AtomIR::BinaryInst* inst) {
    auto dest = inst->getResult();
    auto operand1 = inst->getOperand1();
    auto operand2 = inst->getOperand2();
    if (inst->isIntInst()) {
        _value2reg[dest] = emitIntBinaryInst(inst->getInstType(), operand1, operand2);
    } else {
        _value2reg[dest] = emitFloatBinaryInst(inst->getInstType(), operand1, operand2);
    }
}

void CodeGenerator::emitCondJumpInst(AtomIR::CondJumpInst* inst) {
    Register* src1 = getRegFromValue(inst->getOperand1());
    Register* src2 = getRegFromValue(inst->getOperand2());

    int type;
    switch (inst->getInstType()) {
        case AtomIR::CondJumpInst::INST_JEQ:
            type = CondJumpInst::INST_BEQ;
            break;
        case AtomIR::CondJumpInst::INST_JNE:
            type = CondJumpInst::INST_BNE;
            break;
        case AtomIR::CondJumpInst::INST_JLT:
            type = CondJumpInst::INST_BLT;
            break;
        case AtomIR::CondJumpInst::INST_JLE:
            type = CondJumpInst::INST_BGE;
            std::swap(src1, src2);
            break;
        case AtomIR::CondJumpInst::INST_JGT:
            type = CondJumpInst::INST_BLT;
            std::swap(src1, src2);
            break;
        case AtomIR::CondJumpInst::INST_JGE:
            type = CondJumpInst::INST_BGE;
        default:
            break;
    }
    _currentBasicBlock->addInstruction(new CondJumpInst(type, src1, src2, _atomBB2asmBB[inst->getTureBB()]));
    _currentBasicBlock->addInstruction(new JumpInst(_atomBB2asmBB[inst->getFalseBB()]));
}

Register* CodeGenerator::emitIntBinaryInst(int instType, AtomIR::Value* operand1, AtomIR::Value* operand2) {
    Register* src1;
    Register* src2 = nullptr;
    int imm;
    bool needXor = false;
    if (operand1->isConst()) {
        imm = static_cast<AtomIR::ConstantInt*>(operand1)->getConstValue();
        switch (instType) {
            case AtomIR::BinaryInst::INST_SUB:
            case AtomIR::BinaryInst::INST_MUL:
            case AtomIR::BinaryInst::INST_DIV:
            case AtomIR::BinaryInst::INST_MOD:
            case AtomIR::BinaryInst::INST_LT: {
                src1 = loadConstInt(imm);
                src2 = getRegFromValue(operand2);
                break;
            }
            case AtomIR::BinaryInst::INST_LE: {
                src1 = loadConstInt(imm - 1);
                src2 = getRegFromValue(operand2);
                break;
            }
            case AtomIR::BinaryInst::INST_GT:
                src1 = getRegFromValue(operand2);
                break;
            case AtomIR::BinaryInst::INST_GE:
                imm += 1;
                src1 = getRegFromValue(operand2);
                break;
            case AtomIR::BinaryInst::INST_EQ:
            case AtomIR::BinaryInst::INST_NE:
                imm = -imm;
                src1 = getRegFromValue(operand2);
                break;
            default:
                src1 = getRegFromValue(operand2);
                break;
        }
    } else if (operand2->isConst()) {
        imm = static_cast<AtomIR::ConstantInt*>(operand2)->getConstValue();
        switch (instType) {
            case AtomIR::BinaryInst::INST_SUB:
                imm = -imm;
                src1 = getRegFromValue(operand1);
                break;
            case AtomIR::BinaryInst::INST_MUL:
            case AtomIR::BinaryInst::INST_DIV:
            case AtomIR::BinaryInst::INST_MOD:
                src1 = getRegFromValue(operand1);
                src2 = loadConstInt(imm);
                break;
            case AtomIR::BinaryInst::INST_LE:
                imm += 1;
                src1 = getRegFromValue(operand1);
                break;
            case AtomIR::BinaryInst::INST_GT:
                src1 = loadConstInt(imm);
                src2 = getRegFromValue(operand1);
                break;
            case AtomIR::BinaryInst::INST_GE:
                src1 = loadConstInt(imm - 1);
                src2 = getRegFromValue(operand1);
                break;
            case AtomIR::BinaryInst::INST_EQ:
            case AtomIR::BinaryInst::INST_NE:
                imm = -imm;
                src1 = getRegFromValue(operand1);
                break;
            default:
                src1 = getRegFromValue(operand1);
                break;
        }
    } else {
        switch (instType) {
            case AtomIR::BinaryInst::INST_LE:
                needXor = true;
                src1 = getRegFromValue(operand2);
                src2 = getRegFromValue(operand1);
                break;
            case AtomIR::BinaryInst::INST_GT:
                src1 = getRegFromValue(operand2);
                src2 = getRegFromValue(operand1);
                break;
            case AtomIR::BinaryInst::INST_GE:
                needXor = true;
            default:
                src1 = getRegFromValue(operand1);
                src2 = getRegFromValue(operand2);
                break;
        }
    }

    Instruction* binaryInst;
    switch (instType) {
        case AtomIR::BinaryInst::INST_ADD:
            if (src2) {
                binaryInst = new BinaryInst(BinaryInst::INST_ADDW, src1, src2);
            } else {
                binaryInst = new BinaryInst(BinaryInst::INST_ADDIW, src1, imm);
            }
            break;
        case AtomIR::BinaryInst::INST_SUB:
            if (src2) {
                binaryInst = new BinaryInst(BinaryInst::INST_SUBW, src1, src2);
            } else {
                binaryInst = new BinaryInst(BinaryInst::INST_ADDIW, src1, imm);
            }
            break;
        case AtomIR::BinaryInst::INST_MUL:
            binaryInst = new BinaryInst(BinaryInst::INST_MULW, src1, src2);
            break;
        case AtomIR::BinaryInst::INST_DIV:
            binaryInst = new BinaryInst(BinaryInst::INST_DIVW, src1, src2);
            break;
        case AtomIR::BinaryInst::INST_MOD:
            binaryInst = new BinaryInst(BinaryInst::INST_REMW, src1, src2);
            break;
        case AtomIR::BinaryInst::INST_LT:
        case AtomIR::BinaryInst::INST_LE:
        case AtomIR::BinaryInst::INST_GT:
        case AtomIR::BinaryInst::INST_GE:
            if (src2) {
                binaryInst = new BinaryInst(BinaryInst::INST_SLT, src1, src2);
            } else {
                binaryInst = new BinaryInst(BinaryInst::INST_SLTI, src1, imm);
            }
            break;
        case AtomIR::BinaryInst::INST_EQ:
        case AtomIR::BinaryInst::INST_NE:
            if (src2) {
                binaryInst = new BinaryInst(BinaryInst::INST_XOR, src1, src2);
            } else {
                binaryInst = new BinaryInst(BinaryInst::INST_ADDI, src1, imm);
            }
            break;
        default:
            assert(0 && "unsupported yet");
            break;
    }
    _currentBasicBlock->addInstruction(binaryInst);
    auto dest = binaryInst->getDest();
    if (needXor) {
        auto xorInst = new BinaryInst(BinaryInst::INST_XORI, binaryInst->getDest(), 1);
        _currentBasicBlock->addInstruction(xorInst);
        dest = xorInst->getDest();
    }
    if (instType == AtomIR::BinaryInst::INST_EQ) {
        auto seqz = new UnaryInst(UnaryInst::INST_SEQZ, binaryInst->getDest());
        _currentBasicBlock->addInstruction(seqz);
        dest = seqz->getDest();
    } else if (instType == AtomIR::BinaryInst::INST_NE) {
        auto snez = new UnaryInst(UnaryInst::INST_SNEZ, binaryInst->getDest());
        _currentBasicBlock->addInstruction(snez);
        dest = snez->getDest();
    }

    return dest;
}

Register* CodeGenerator::emitFloatBinaryInst(int instType, AtomIR::Value* operand1, AtomIR::Value* operand2) {
    Register* src1 = getRegFromValue(operand1);
    Register* src2 = getRegFromValue(operand2);
    Instruction* binaryInst;
    switch (instType) {
        case AtomIR::BinaryInst::INST_ADD:
            binaryInst = new BinaryInst(BinaryInst::INST_FADD_S, src1, src2);
            break;
        case AtomIR::BinaryInst::INST_SUB:
            binaryInst = new BinaryInst(BinaryInst::INST_FSUB_S, src1, src2);
            break;
        case AtomIR::BinaryInst::INST_MUL:
            binaryInst = new BinaryInst(BinaryInst::INST_FMUL_S, src1, src2);
            break;
        case AtomIR::BinaryInst::INST_DIV:
            binaryInst = new BinaryInst(BinaryInst::INST_FDIV_S, src1, src2);
            break;
        case AtomIR::BinaryInst::INST_LT:
            binaryInst = new BinaryInst(BinaryInst::INST_FSLT_S, src1, src2);
            break;
        case AtomIR::BinaryInst::INST_LE:
            binaryInst = new BinaryInst(BinaryInst::INST_FSLE_S, src1, src2);
            break;
        case AtomIR::BinaryInst::INST_GT:
            std::swap(src1, src2);
            binaryInst = new BinaryInst(BinaryInst::INST_FSLT_S, src1, src2);
            break;
        case AtomIR::BinaryInst::INST_GE:
            std::swap(src1, src2);
            binaryInst = new BinaryInst(BinaryInst::INST_FSLE_S, src1, src2);
            break;
        case AtomIR::BinaryInst::INST_EQ:
        case AtomIR::BinaryInst::INST_NE:
            binaryInst = new BinaryInst(BinaryInst::INST_FSEQ_S, src1, src2);
            break;
        default:
            assert(0 && "unsupported yet");
            break;
    }
    _currentBasicBlock->addInstruction(binaryInst);
    if (instType == AtomIR::BinaryInst::INST_NE) {
        _currentBasicBlock->addInstruction(
            new UnaryInst(UnaryInst::INST_SEQZ, binaryInst->getDest(), binaryInst->getDest()));
    }
    return binaryInst->getDest();
}

Register* CodeGenerator::loadConstInt(int value) {
    auto li = new ImmInst(ImmInst::INST_LI, value);
    _currentBasicBlock->addInstruction(li);
    return li->getDest();
}

Register* CodeGenerator::loadConstFloat(float value) {
    std::string lable;
    if (_float2lable.find(value) != _float2lable.end()) {
        lable = _float2lable[value];
    } else {
        lable = ".LC" + std::to_string(_float2lable.size());
        _float2lable.insert({value, lable});
    }
    auto la = new LoadGlobalAddrInst(_float2lable[value]);
    _currentBasicBlock->addInstruction(la);
    auto flw = new LoadInst(LoadInst::INST_FLW, la->getDest(), 0);
    _currentBasicBlock->addInstruction(flw);
    return flw->getDest();
}

Register* CodeGenerator::loadConst(AtomIR::Constant* value) {
    if (value->isInt()) {
        return loadConstInt(static_cast<AtomIR::ConstantInt*>(value)->getConstValue());
    } else {
        return loadConstFloat(static_cast<AtomIR::ConstantFloat*>(value)->getConstValue());
    }
}

Register* CodeGenerator::getRegFromValue(AtomIR::Value* value) {
    if (value->isConst()) {
        return loadConst((AtomIR::Constant*)value);
    }

    if (value->isGlobal()) {
        auto la = new LoadGlobalAddrInst(value->getName());
        _currentBasicBlock->addInstruction(la);
        return la->getDest();
    }

    if (value->getType()->isPointerType() && _value2reg.find(value) == _value2reg.end()) {
        return Register::S0;
    }

    return _value2reg[value];
}

}  // namespace RISCV
}  // namespace ATC
