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

CodeGenerator::CodeGenerator() {
    _ra = new Register();
    _ra->setName("ra");
    _ra->setIsFixed(true);

    _s0 = new Register();
    _s0->setName("s0");
    _s0->setIsFixed(true);

    _sp = new Register();
    _sp->setName("sp");
    _sp->setIsFixed(true);

    _zero = new Register();
    _zero->setName("zero");
    _zero->setIsFixed(true);

    for (int i = 0; i < 8; i++) {
        auto argReg = new Register();
        argReg->setName("a" + std::to_string(i));
        argReg->setIsFixed(true);
        _intArgReg.push_back(argReg);

        argReg = new Register();
        argReg->setName("fa" + std::to_string(i));
        argReg->setIsFixed(true);
        _floatArgReg.push_back(argReg);
    }
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

    int i = 0;
    for (auto param : function->getParams()) {
        _value2reg[param] = _intArgReg[i++];
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

    RegAllocator regAllocator(_currentFunction, true);
    regAllocator.run();

    entryBB->addInstruction(new BinaryInst(BinaryInst::INST_ADDI, _sp, _sp, _offset));
    if (function->hasFunctionCall()) {
        entryBB->addInstruction(new StoreInst(StoreInst::INST_SD, _ra, _sp, -_offset - 8));
        entryBB->addInstruction(new StoreInst(StoreInst::INST_SD, _s0, _sp, -_offset - 16));
        retBB->addInstruction(new LoadInst(LoadInst::INST_LD, _ra, _sp, -_offset - 8));
        retBB->addInstruction(new LoadInst(LoadInst::INST_LD, _s0, _sp, -_offset - 16));
    } else {
        entryBB->addInstruction(new StoreInst(StoreInst::INST_SD, _s0, _sp, -_offset - 8));
        retBB->addInstruction(new LoadInst(LoadInst::INST_LD, _s0, _sp, -_offset - 8));
    }
    entryBB->addInstruction(new BinaryInst(BinaryInst::INST_ADDI, _s0, _sp, -_offset));
    retBB->addInstruction(new BinaryInst(BinaryInst::INST_ADDI, _sp, _sp, -_offset));
    retBB->addInstruction(new ReturnInst());

    _contend << "\t.globl\t" << function->getName() << endl;
    _contend << "\t.p2align\t1" << endl;
    _contend << "\t.type\t" << function->getName() << ",@function" << endl;
    _contend << "." << function->getName() << ":";
    // append completed code into content
    for (auto bb : _currentFunction->getBasicBlocks()) {
        _contend << endl << bb->toString();
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
        case AtomIR::ID_BITCAST_INST:  //  nothing need to do
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
    _value2reg[inst->getResult()] = _s0;
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
    _currentBasicBlock->addInstruction(new FunctionCallInst(inst->getFuncName()));
    if (inst->getResult()) {
        _value2reg[inst->getResult()] = new Register();
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

void CodeGenerator::emitRetInst(AtomIR::ReturnInst* inst) { _currentBasicBlock->addInstruction(new JumpInst(retBB)); }

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
        case AtomIR::UnaryInst::INST_ITOF:
            unaryInst = new UnaryInst(UnaryInst::INST_FCVT_S_W, src1);
            break;
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
    auto dest = binaryInst->getDest();

    switch (instType) {
        case AtomIR::BinaryInst::INST_LT:
        case AtomIR::BinaryInst::INST_LE:
        case AtomIR::BinaryInst::INST_GT:
        case AtomIR::BinaryInst::INST_GE:
        case AtomIR::BinaryInst::INST_EQ: {
            auto zeroBB = new BasicBlock();
            _currentFunction->addBasicBlock(zeroBB);
            auto afterBB = new BasicBlock();
            _currentFunction->addBasicBlock(afterBB);
            auto beq = new CondJumpInst(CondJumpInst::INST_BEQ, dest, _zero, zeroBB);
            _currentBasicBlock->addInstruction(beq);
            dest = loadConstFloat(1);
            _currentBasicBlock->addInstruction(new JumpInst(afterBB));
            zeroBB->addInstruction(new UnaryInst(UnaryInst::INST_FMV_W_X, dest, _zero));
            _currentBasicBlock = afterBB;
            break;
        }
        case AtomIR::BinaryInst::INST_NE: {
            auto zeroBB = new BasicBlock();
            _currentFunction->addBasicBlock(zeroBB);
            auto afterBB = new BasicBlock();
            _currentFunction->addBasicBlock(afterBB);
            auto bne = new CondJumpInst(CondJumpInst::INST_BNE, dest, _zero, zeroBB);
            _currentBasicBlock->addInstruction(bne);
            dest = loadConstFloat(1);
            _currentBasicBlock->addInstruction(new JumpInst(afterBB));
            zeroBB->addInstruction(new UnaryInst(UnaryInst::INST_FMV_W_X, dest, _zero));
            _currentBasicBlock = afterBB;
        }
        default:
            break;
    }
    return dest;
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
        return _s0;
    }

    return _value2reg[value];
}

}  // namespace RISCV
}  // namespace ATC
