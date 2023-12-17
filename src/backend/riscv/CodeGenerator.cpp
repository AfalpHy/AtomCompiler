#include "riscv/CodeGenerator.h"

#include <assert.h>

#include "atomIR/Instruction.h"
#include "atomIR/Module.h"
#include "riscv/BasicBlock.h"
#include "riscv/Function.h"

namespace ATC {
namespace RISCV {

using std::endl;

int BasicBlock::Index = 0;
int Register::Index = 0;

CodeGenerator::CodeGenerator() {
    _sp = new Register();
    _sp->setName("sp");
    _sp->setFixed();
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
    _offset = 0;
    _value2offset.clear();

    _currentFunction = new Function();

    // prepare the BasicBlock lable
    bool first = true;
    for (auto bb : function->getBasicBlocks()) {
        // first block does not need the label
        if (first) {
            first = false;
            continue;
        }
        _bb2lable[bb] = ".LBB" + std::to_string(BasicBlock::Index++);
    }

    for (auto bb : function->getBasicBlocks()) {
        emitBasicBlock(bb);
    }
    /// TODO: modify the instruct

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
    _currentBasicBlock = new BasicBlock(_bb2lable[basicBlock]);

    for (auto inst : basicBlock->getInstructionList()) {
        emitInstruction(inst);
    }
    _currentFunction->addBasicBlock(_currentBasicBlock);
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
            _currentBasicBlock->addInstruction(new JumpInst(_bb2lable[jumpInst->getTargetBB()]));
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
    _value2offset[inst->getResult()] = _offset;
    auto type = inst->getResult()->getType()->getBaseType();
    _offset += type->getByteLen();
    _value2reg[inst->getResult()] = _sp;
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

void CodeGenerator::emitRetInst(AtomIR::ReturnInst* inst) { _currentBasicBlock->addInstruction(new ReturnInst()); }

void CodeGenerator::emitUnaryInst(AtomIR::UnaryInst* inst) {
    Register* src1 = getRegFromValue(inst->getOperand());
    Instruction* unaryInst = nullptr;
    switch (inst->getInstType()) {
        case AtomIR::UnaryInst::INST_LOAD: {
            int offset = inst->getOperand()->isGlobal() ? 0 : _value2offset[inst->getOperand()];
            unaryInst = new LoadInst(LoadInst::INST_LW, src1, offset);
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
    Register* src1;
    Register* src2;
    int imm;
    bool isImmInst = false;
    bool needXor = false;
    if (operand1->isConst()) {
        auto constOp1 = (AtomIR::Constant*)operand1;
        if (constOp1->isInt()) {
            imm = static_cast<AtomIR::ConstantInt*>(constOp1)->getConstValue();
            switch (inst->getInstType()) {
                case AtomIR::BinaryInst::INST_SUB:
                case AtomIR::BinaryInst::INST_DIV:
                case AtomIR::BinaryInst::INST_MOD:
                case AtomIR::BinaryInst::INST_LT: {
                    src1 = loadConstInt(imm);
                    src2 = getRegFromValue(inst->getOperand2());
                    break;
                }
                case AtomIR::BinaryInst::INST_LE: {
                    src1 = loadConstInt(imm - 1);
                    src2 = getRegFromValue(inst->getOperand2());
                    break;
                }
                case AtomIR::BinaryInst::INST_GT:
                    isImmInst = true;
                    src1 = getRegFromValue(inst->getOperand2());
                    break;
                case AtomIR::BinaryInst::INST_GE:
                    isImmInst = true;
                    imm += 1;
                    src1 = getRegFromValue(inst->getOperand2());
                    break;
                case AtomIR::BinaryInst::INST_EQ:
                case AtomIR::BinaryInst::INST_NE:
                    isImmInst = true;
                    imm = -imm;
                    src1 = getRegFromValue(inst->getOperand2());
                    break;
                default:
                    isImmInst = true;
                    src1 = getRegFromValue(inst->getOperand2());
                    break;
            }
        } else {
        }
    } else if (operand2->isConst()) {
        auto constOp2 = (AtomIR::Constant*)operand2;
        if (constOp2->isInt()) {
            imm = static_cast<AtomIR::ConstantInt*>(constOp2)->getConstValue();
            switch (inst->getInstType()) {
                case AtomIR::BinaryInst::INST_SUB:
                    isImmInst = true;
                    imm = -imm;
                    src1 = getRegFromValue(inst->getOperand1());
                    break;
                case AtomIR::BinaryInst::INST_MUL:
                case AtomIR::BinaryInst::INST_DIV:
                case AtomIR::BinaryInst::INST_MOD:
                    isImmInst = false;
                    src1 = getRegFromValue(inst->getOperand1());
                    src2 = loadConstInt(imm);
                    break;
                case AtomIR::BinaryInst::INST_LE:
                    isImmInst = true;
                    imm += 1;
                    src1 = getRegFromValue(inst->getOperand1());
                    break;
                case AtomIR::BinaryInst::INST_GT:
                    isImmInst = false;
                    src1 = loadConstInt(imm);
                    src2 = getRegFromValue(inst->getOperand1());
                    break;
                case AtomIR::BinaryInst::INST_GE:
                    isImmInst = false;
                    src1 = loadConstInt(imm - 1);
                    src2 = getRegFromValue(inst->getOperand1());
                    break;
                case AtomIR::BinaryInst::INST_EQ:
                case AtomIR::BinaryInst::INST_NE:
                    isImmInst = true;
                    imm = -imm;
                    src1 = getRegFromValue(inst->getOperand1());
                    break;
                default:
                    isImmInst = true;
                    src1 = getRegFromValue(inst->getOperand1());
                    break;
            }
        } else {
        }
    } else {
        switch (inst->getInstType()) {
            case AtomIR::BinaryInst::INST_LE:
                needXor = true;
                src1 = getRegFromValue(inst->getOperand2());
                src2 = getRegFromValue(inst->getOperand1());
                break;
            case AtomIR::BinaryInst::INST_GT:
                src1 = getRegFromValue(inst->getOperand2());
                src2 = getRegFromValue(inst->getOperand1());
                break;
            case AtomIR::BinaryInst::INST_GE:
                needXor = true;
            default:
                src1 = getRegFromValue(inst->getOperand1());
                src2 = getRegFromValue(inst->getOperand2());
                break;
        }
    }

    Instruction* binaryInst;
    switch (inst->getInstType()) {
        case AtomIR::BinaryInst::INST_ADD:
            if (isImmInst) {
                binaryInst = new BinaryInst(BinaryInst::INST_ADDI, src1, imm);
            } else {
                binaryInst = new BinaryInst(BinaryInst::INST_ADD, src1, src2);
            }
            break;
        case AtomIR::BinaryInst::INST_SUB:
            if (isImmInst) {
                binaryInst = new BinaryInst(BinaryInst::INST_ADDI, src1, imm);
            } else {
                binaryInst = new BinaryInst(BinaryInst::INST_SUB, src1, src2);
            }
            break;
        case AtomIR::BinaryInst::INST_MUL:
            binaryInst = new BinaryInst(BinaryInst::INST_MUL, src1, src2);
            break;
        case AtomIR::BinaryInst::INST_DIV:
            binaryInst = new BinaryInst(BinaryInst::INST_DIV, src1, src2);
            break;
        case AtomIR::BinaryInst::INST_MOD:
            binaryInst = new BinaryInst(BinaryInst::INST_REM, src1, src2);
            break;
        case AtomIR::BinaryInst::INST_LT:
        case AtomIR::BinaryInst::INST_LE:
        case AtomIR::BinaryInst::INST_GT:
        case AtomIR::BinaryInst::INST_GE:
            if (isImmInst) {
                binaryInst = new BinaryInst(BinaryInst::INST_SLTI, src1, imm);
            } else {
                binaryInst = new BinaryInst(BinaryInst::INST_SLT, src1, src2);
            }
            break;
        case AtomIR::BinaryInst::INST_EQ:
        case AtomIR::BinaryInst::INST_NE:
            if (isImmInst) {
                binaryInst = new BinaryInst(BinaryInst::INST_ADDI, src1, imm);
            } else {
                binaryInst = new BinaryInst(BinaryInst::INST_XOR, src1, src2);
            }
            break;
        default:
            assert(0 && "unsupported yet");
            break;
    }
    _currentBasicBlock->addInstruction(binaryInst);
    _value2reg[dest] = binaryInst->getDest();
    if (needXor) {
        auto xorInst = new BinaryInst(BinaryInst::INST_XORI, binaryInst->getDest(), 1);
        _currentBasicBlock->addInstruction(xorInst);
        _value2reg[dest] = xorInst->getDest();
    }
    switch (inst->getInstType()) {
        case AtomIR::BinaryInst::INST_EQ: {
            auto seqz = new UnaryInst(UnaryInst::INST_SEQZ, binaryInst->getDest());
            _currentBasicBlock->addInstruction(seqz);
            _value2reg[dest] = seqz->getDest();
            break;
        }
        case AtomIR::BinaryInst::INST_NE: {
            auto snez = new UnaryInst(UnaryInst::INST_SNEZ, binaryInst->getDest());
            _currentBasicBlock->addInstruction(snez);
            _value2reg[dest] = snez->getDest();
            break;
        }
        default:
            break;
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
    _currentBasicBlock->addInstruction(new CondJumpInst(type, src1, src2, _bb2lable[inst->getTureBB()]));
    _currentBasicBlock->addInstruction(new JumpInst(_bb2lable[inst->getFalseBB()]));
}

Register* CodeGenerator::loadConstInt(int value) {
    auto li = new ImmInst(ImmInst::INST_LI, value);
    _currentBasicBlock->addInstruction(li);
    return li->getDest();
}

Register* CodeGenerator::loadConstFloat(float value) { return nullptr; }

Register* CodeGenerator::loadConst(AtomIR::Constant* value) {
    if (value->isInt()) {
        return loadConstInt(static_cast<AtomIR::ConstantInt*>(value)->getConstValue());
    } else {
        return nullptr;
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
        return _sp;
    }

    return _value2reg[value];
}

}  // namespace RISCV
}  // namespace ATC
