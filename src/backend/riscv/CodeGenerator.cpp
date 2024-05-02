#include "riscv/CodeGenerator.h"

#include <assert.h>

#include "IR/Instruction.h"
#include "IR/Module.h"
#include "riscv/BasicBlock.h"
#include "riscv/Function.h"
#include "riscv/RegAllocator.h"

namespace ATC {
namespace RISCV {

using std::endl;

int BasicBlock::Index = 0;
int Register::Index = 0;

std::vector<Register*> Function::CallerSavedRegs;
std::vector<Register*> Function::CalleeSavedRegs;

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

        argReg = new Register(false);
        argReg->setName("fa" + std::to_string(i));
        argReg->setIsFixed(true);
        Register::FloatArgReg.push_back(argReg);
        Function::CallerSavedRegs.push_back(argReg);
    }

    for (int i = 0; i < 7; i++) {
        auto tmpReg = new Register();
        tmpReg->setName("t" + std::to_string(i));
        tmpReg->setIsFixed(true);
        Function::CallerSavedRegs.push_back(tmpReg);
    }
    for (int i = 0; i < 12; i++) {
        auto tmpReg = new Register(false);
        tmpReg->setName("ft" + std::to_string(i));
        tmpReg->setIsFixed(true);
        Function::CallerSavedRegs.push_back(tmpReg);
    }

    for (int i = 0; i < 12; i++) {
        auto tmpReg = new Register();
        tmpReg->setName("s" + std::to_string(i));
        tmpReg->setIsFixed(true);
        Function::CalleeSavedRegs.push_back(tmpReg);
    }
    for (int i = 0; i < 12; i++) {
        auto tmpReg = new Register(false);
        tmpReg->setName("fs" + std::to_string(i));
        tmpReg->setIsFixed(true);
        Function::CalleeSavedRegs.push_back(tmpReg);
    }
}

void CodeGenerator::dump() { std::cout << _contend.str() << endl; }

void CodeGenerator::print(std::ofstream& os) { os << _contend.str() << endl; }

void CodeGenerator::emitModule(IR::Module* module) {
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
        // "aw",@progbits is important, the program's result is wrong without it
        _contend << "\t.section\t.sdata,\"aw\",@progbits" << endl;
        _contend << "\t.p2align\t2" << endl;
    }
    for (auto& [value, lable] : _float2lable) {
        _contend << lable << ":" << endl;
        _contend << "\t.word\t" << std::to_string(*(int*)(&value)) << endl;
    }
}

void CodeGenerator::emitGlobalVariable(IR::GloabalVariable* var) {
    _contend << "\t.type\t" << var->getName() << ",@object" << endl;
    _contend << "\t.globl\t" << var->getName() << endl;
    _contend << "\t.p2align\t2" << endl;
    _contend << var->getName() << ":" << endl;
    int size = 0;
    if (auto arrayValue = dynamic_cast<IR::ArrayValue*>(var->getInitialValue())) {
        for (auto& item : arrayValue->getElements()) {
            if (item.second.empty()) {
                size += item.first * 4;
                _contend << "\t.zero\t" << std::to_string(item.first * 4) << endl;
            } else {
                for (auto& element : item.second) {
                    size += 4;
                    _contend << "\t.word\t" << static_cast<IR::Constant*>(element)->getLiteralStr() << endl;
                }
            }
        }
    } else {
        size = 4;
        _contend << "\t.word\t" << static_cast<IR::Constant*>(var->getInitialValue())->getLiteralStr() << endl;
    }
    _contend << "\t.size\t" << var->getName() << ", " << std::to_string(size) << endl << endl;
}

void CodeGenerator::emitFunction(IR::Function* function) {
    _currentFunction = new Function(function->getName());

    std::set<Register*> tmpNeedPushRegs;

    do {
        tmpNeedPushRegs = _currentFunction->getNeedPushRegs();

        // reset
        _offset = 0;
        _value2reg.clear();
        _value2offset.clear();
        _paramInStack.clear();
        _atomBB2asmBB.clear();
        _maxPassParamsStackOffset = 0;
        _currentFunction->getMutableBasicBlocks().clear();

        if (function->hasFunctionCall()) {
            // save ra and s0
            _offset = -16;
        } else {
            // save s0
            _offset = -8;
        }
        _offset -= tmpNeedPushRegs.size() * 8;

        int intOrder = 0;
        int floatOrder = 0;
        for (auto param : function->getParams()) {
            if (param->getType()->isIntType()) {
                if (intOrder < 8) {
                    _value2reg[param] = Register::IntArgReg[intOrder++];
                } else {
                    _paramInStack.insert(param);
                }
            } else {
                if (floatOrder < 8) {
                    _value2reg[param] = Register::FloatArgReg[floatOrder++];
                } else {
                    _paramInStack.insert(param);
                }
            }
        }
        // prepare the BasicBlocks
        _entryBB = new BasicBlock();
        for (auto bb : function->getBasicBlocks()) {
            _atomBB2asmBB[bb] = new BasicBlock();
        }
        _retBB = new BasicBlock("." + function->getName() + "_ret");

        _currentFunction->addBasicBlock(_entryBB);
        for (auto bb : function->getBasicBlocks()) {
            emitBasicBlock(bb);
        }
        _currentFunction->addBasicBlock(_retBB);

        RegAllocator regAllocator(_currentFunction, _offset, true);
        regAllocator.run();
    } while (tmpNeedPushRegs != _currentFunction->getNeedPushRegs());

    _offset -= _maxPassParamsStackOffset;
    // The stack is aligned to 16 bytes
    if (_offset % 16 != 0) {
        _offset = (_offset - 15) / 16 * 16;
    }
    if (_offset >= -2048) {
        _entryBB->addInstruction(new BinaryInst(BinaryInst::INST_ADDI, Register::Sp, Register::Sp, _offset));
        int pushRegOffset;
        if (function->hasFunctionCall()) {
            pushRegOffset = -_offset - 24;
            _entryBB->addInstruction(new StoreInst(StoreInst::INST_SD, Register::Ra, Register::Sp, -_offset - 8));
            _entryBB->addInstruction(new StoreInst(StoreInst::INST_SD, Register::S0, Register::Sp, -_offset - 16));
            _retBB->addInstruction(new LoadInst(LoadInst::INST_LD, Register::Ra, Register::Sp, -_offset - 8));
            _retBB->addInstruction(new LoadInst(LoadInst::INST_LD, Register::S0, Register::Sp, -_offset - 16));
        } else {
            pushRegOffset = -_offset - 16;
            _entryBB->addInstruction(new StoreInst(StoreInst::INST_SD, Register::S0, Register::Sp, -_offset - 8));
            _retBB->addInstruction(new LoadInst(LoadInst::INST_LD, Register::S0, Register::Sp, -_offset - 8));
        }
        for (auto reg : _currentFunction->getNeedPushRegs()) {
            if (reg->isIntReg()) {
                _entryBB->addInstruction(new StoreInst(StoreInst::INST_SD, reg, Register::Sp, pushRegOffset));
                _retBB->addInstruction(new LoadInst(LoadInst::INST_LD, reg, Register::Sp, pushRegOffset));
            } else {
                _entryBB->addInstruction(new StoreInst(StoreInst::INST_FSD, reg, Register::Sp, pushRegOffset));
                _retBB->addInstruction(new LoadInst(LoadInst::INST_FLD, reg, Register::Sp, pushRegOffset));
            }
            pushRegOffset -= 8;
        }

        _entryBB->addInstruction(new BinaryInst(BinaryInst::INST_ADDI, Register::S0, Register::Sp, -_offset));
        _retBB->addInstruction(new BinaryInst(BinaryInst::INST_ADDI, Register::Sp, Register::Sp, -_offset));
    } else {
        // 2032 avoid to ues the num 2048
        _entryBB->addInstruction(new BinaryInst(BinaryInst::INST_ADDI, Register::Sp, Register::Sp, -2032));
        _currentBasicBlock = _retBB;
        auto tmpImm = loadConstInt(-_offset - 2032);
        tmpImm->setName("t0");
        tmpImm->setIsFixed(true);
        _currentBasicBlock->addInstruction(new BinaryInst(BinaryInst::INST_ADD, Register::Sp, Register::Sp, tmpImm));
        int pushRegOffset;
        if (function->hasFunctionCall()) {
            pushRegOffset = 2008;
            _entryBB->addInstruction(new StoreInst(StoreInst::INST_SD, Register::Ra, Register::Sp, 2024));
            _entryBB->addInstruction(new StoreInst(StoreInst::INST_SD, Register::S0, Register::Sp, 2016));
            _retBB->addInstruction(new LoadInst(LoadInst::INST_LD, Register::Ra, Register::Sp, 2024));
            _retBB->addInstruction(new LoadInst(LoadInst::INST_LD, Register::S0, Register::Sp, 2016));
        } else {
            pushRegOffset = 2016;
            _entryBB->addInstruction(new StoreInst(StoreInst::INST_SD, Register::S0, Register::Sp, 2024));
            _retBB->addInstruction(new LoadInst(LoadInst::INST_LD, Register::S0, Register::Sp, 2024));
        }
        for (auto reg : _currentFunction->getNeedPushRegs()) {
            if (reg->isIntReg()) {
                _entryBB->addInstruction(new StoreInst(StoreInst::INST_SD, reg, Register::Sp, pushRegOffset));
                _retBB->addInstruction(new LoadInst(LoadInst::INST_LD, reg, Register::Sp, pushRegOffset));
            } else {
                _entryBB->addInstruction(new StoreInst(StoreInst::INST_FSD, reg, Register::Sp, pushRegOffset));
                _retBB->addInstruction(new LoadInst(LoadInst::INST_FLD, reg, Register::Sp, pushRegOffset));
            }
            pushRegOffset -= 8;
        }

        _entryBB->addInstruction(new BinaryInst(BinaryInst::INST_ADDI, Register::S0, Register::Sp, 2032));

        _currentBasicBlock = _entryBB;
        tmpImm = loadConstInt(_offset + 2032);
        tmpImm->setName("t0");
        tmpImm->setIsFixed(true);
        _currentBasicBlock->addInstruction(new BinaryInst(BinaryInst::INST_ADD, Register::Sp, Register::Sp, tmpImm));
        _retBB->addInstruction(new BinaryInst(BinaryInst::INST_ADDI, Register::Sp, Register::Sp, 2032));
    }

    _retBB->addInstruction(new ReturnInst());

    // remove redundant jump and lable
    for (auto begin = _currentFunction->getBasicBlocks().begin(); begin != _currentFunction->getBasicBlocks().end();
         begin++) {
        auto tmpBB = *begin;
        if (tmpBB->getPredecessors().empty()) {
            tmpBB->setIsNeedLable(false);
        }
        auto lastInst = tmpBB->getInstructionList().back();
        if (lastInst->getClassId() == ID_JUMP_INST) {
            BasicBlock* targetBB = static_cast<JumpInst*>(lastInst)->getTargetBB();
            auto nextBBIter = begin;
            auto nextBB = *++nextBBIter;
            if (nextBB == targetBB) {
                tmpBB->getMutableInstructionList().pop_back();
                if (targetBB->getPredecessors().size() == 1) {
                    targetBB->setIsNeedLable(false);
                }
            }
        }
    }
    _contend << _currentFunction->toString();
}

void CodeGenerator::emitBasicBlock(IR::BasicBlock* basicBlock) {
    _currentBasicBlock = _atomBB2asmBB[basicBlock];
    _currentFunction->addBasicBlock(_currentBasicBlock);
    for (auto inst : basicBlock->getInstructionList()) {
        emitInstruction(inst);
    }
}

void CodeGenerator::emitInstruction(IR::Instruction* inst) {
    if (inst->isDead()) {
        return;
    }
    switch (inst->getClassId()) {
        case IR::ID_ALLOC_INST:
            emitAllocInst((IR::AllocInst*)inst);
            break;
        case IR::ID_STORE_INST:
            emitStoreInst((IR::StoreInst*)inst);
            break;
        case IR::ID_FUNCTION_CALL_INST:
            emitFunctionCallInst((IR::FunctionCallInst*)inst);
            break;
        case IR::ID_GET_ELEMENT_PTR_INST:
            emitGEPInst((IR::GetElementPtrInst*)inst);
            break;
        case IR::ID_BITCAST_INST:
            emitBitCastInst((IR::BitCastInst*)inst);
            break;
        case IR::ID_RETURN_INST:
            emitRetInst((IR::ReturnInst*)inst);
            break;
        case IR::ID_UNARY_INST:
            emitUnaryInst((IR::UnaryInst*)inst);
            break;
        case IR::ID_BINARY_INST:
            emitBinaryInst((IR::BinaryInst*)inst);
            break;
        case IR::ID_JUMP_INST: {
            IR::JumpInst* jumpInst = (IR::JumpInst*)inst;
            _currentBasicBlock->addInstruction(new JumpInst(_atomBB2asmBB[jumpInst->getTargetBB()]));
            break;
        }
        case IR::ID_COND_JUMP_INST:
            emitCondJumpInst((IR::CondJumpInst*)inst);
            break;
        default:
            assert(false && "should not reach here");
            break;
    }
}

void CodeGenerator::emitAllocInst(IR::AllocInst* inst) {
    _value2reg[inst->getResult()] = Register::S0;

    auto type = inst->getResult()->getType()->getBaseType();
    if (inst->isAllocForParam()) {
        int intNum = inst->getAllocatedIntParamNum();
        int floatNum = inst->getAllocatedFloatParamNum();
        if (type->isIntType() && intNum > 8) {
            _value2offset[inst->getResult()] = (intNum - 8 + (floatNum > 8 ? floatNum - 8 : 0) - 1) * 8;
            return;
        } else if (!type->isIntType() && floatNum > 8) {
            _value2offset[inst->getResult()] = (floatNum - 8 + (intNum > 8 ? intNum - 8 : 0) - 1) * 8;
            return;
        }
    }
    _offset -= type->getByteLen();
    _value2offset[inst->getResult()] = _offset;
}

void CodeGenerator::emitStoreInst(IR::StoreInst* inst) {
    auto value = inst->getValue();
    /// needn't to store
    if (_paramInStack.find(value) != _paramInStack.end()) {
        return;
    }
    auto dest = inst->getDest();
    Register* src1 = getRegFromValue(value);
    Register* src2 = getRegFromValue(dest);
    int offset = _value2offset[dest];

    int instType;
    auto valueTy = value->getType();
    if (valueTy == IR::Type::getInt32Ty()) {
        instType = StoreInst::INST_SW;
    } else if (valueTy->isPointerType()) {
        instType = StoreInst::INST_SD;
    } else {
        instType = StoreInst::INST_FSW;
    }

    src2 = processIfImmOutOfRange(src2, offset);
    _currentBasicBlock->addInstruction(new StoreInst(instType, src1, src2, offset));
}

void CodeGenerator::emitFunctionCallInst(IR::FunctionCallInst* inst) {
    int intOrder = 0;
    int floatOrder = 0;
    int stackOffset = 0;
    for (auto param : inst->getParams()) {
        auto paramReg = getRegFromValue(param);
        if (param->getType()->isIntType()) {
            if (intOrder < 8) {
                _currentBasicBlock->addInstruction(
                    new UnaryInst(UnaryInst::INST_MV, Register::IntArgReg[intOrder++], paramReg));
            } else {
                auto& mutableInstList = _currentBasicBlock->getMutableInstructionList();
                auto begin = mutableInstList.begin();
                auto end = mutableInstList.end();
                for (; begin != end; begin++) {
                    if ((*begin)->getDest() == paramReg) {
                        ++begin;
                        if (stackOffset > 2047) {
                            auto hi20 = stackOffset >> 12;
                            auto lo12 = stackOffset & 0xfff;
                            if (lo12 > 2047) {
                                lo12 -= 4096;
                                hi20 += 1;
                            }
                            auto lui = new ImmInst(ImmInst::INST_LUI, hi20);
                            mutableInstList.insert(begin, lui);

                            auto add = new BinaryInst(BinaryInst::INST_ADD, Register::Sp, lui->getDest());
                            mutableInstList.insert(begin, add);

                            auto saveParam = new StoreInst(
                                param->getType()->isPointerType() ? StoreInst::INST_SD : StoreInst::INST_SW, paramReg,
                                add->getDest(), lo12);
                            mutableInstList.insert(begin, saveParam);
                        } else {
                            auto saveParam = new StoreInst(
                                param->getType()->isPointerType() ? StoreInst::INST_SD : StoreInst::INST_SW, paramReg,
                                Register::Sp, stackOffset);
                            mutableInstList.insert(begin, saveParam);
                        }
                        break;
                    }
                }
                stackOffset += 8;
            }
        } else {
            if (floatOrder < 8) {
                _currentBasicBlock->addInstruction(
                    new UnaryInst(UnaryInst::INST_FMV_S, Register::FloatArgReg[floatOrder++], paramReg));
            } else {
                auto& mutableInstList = _currentBasicBlock->getMutableInstructionList();
                auto begin = mutableInstList.begin();
                auto end = mutableInstList.end();
                for (; begin != end; begin++) {
                    if ((*begin)->getDest() == paramReg) {
                        ++begin;
                        if (stackOffset > 2047) {
                            auto hi20 = stackOffset >> 12;
                            auto lo12 = stackOffset & 0xfff;
                            if (lo12 > 2047) {
                                lo12 -= 4096;
                                hi20 += 1;
                            }
                            auto lui = new ImmInst(ImmInst::INST_LUI, hi20);
                            mutableInstList.insert(begin, lui);

                            auto add = new BinaryInst(BinaryInst::INST_ADD, Register::Sp, lui->getDest());
                            mutableInstList.insert(begin, add);

                            auto saveParam = new StoreInst(StoreInst::INST_FSW, paramReg, add->getDest(), lo12);
                            mutableInstList.insert(begin, saveParam);
                        } else {
                            auto saveParam = new StoreInst(StoreInst::INST_FSW, paramReg, Register::Sp, stackOffset);
                            mutableInstList.insert(begin, saveParam);
                        }
                        break;
                    }
                }
                stackOffset += 8;
            }
        }
    }
    _maxPassParamsStackOffset = std::max(_maxPassParamsStackOffset, stackOffset);
    Register* dest = nullptr;
    if (inst->getResult()) {
        if (inst->getResult()->getType()->isIntType()) {
            dest = Register::IntArgReg[0];
        } else {
            dest = Register::FloatArgReg[0];
        }
    }
    auto call = new FunctionCallInst(inst->getFuncName(), dest);

    for (int i = 0; i < intOrder; i++) {
        call->addUsedReg(Register::IntArgReg[i]);
    }
    for (int i = 0; i < floatOrder; i++) {
        call->addUsedReg(Register::FloatArgReg[i]);
    }
    _currentBasicBlock->addInstruction(call);
    if (inst->getResult()) {
        Instruction* mv;
        if (inst->getResult()->getType() == IR::Type::getInt32Ty()) {
            mv = new UnaryInst(UnaryInst::INST_MV, Register::IntArgReg[0]);
        } else {
            mv = new UnaryInst(UnaryInst::INST_FMV_S, Register::FloatArgReg[0]);
        }
        _currentBasicBlock->addInstruction(mv);
        _value2reg[inst->getResult()] = mv->getDest();
    }
}

void CodeGenerator::emitGEPInst(IR::GetElementPtrInst* inst) {
    Register* ptr = getRegFromValue(inst->getPtr());
    if (ptr == Register::S0) {
        int offset = _value2offset[inst->getPtr()];
        auto tmp = processIfImmOutOfRange(ptr, offset);
        auto getPtr = new BinaryInst(BinaryInst::INST_ADDI, tmp, offset);
        ptr = getPtr->getDest();
        _currentBasicBlock->addInstruction(getPtr);
    }
    Register* offsetReg;
    auto& indexes = inst->getIndexes();
    if (indexes.size() == 1) {
        int offset = inst->getPtr()->getType()->getBaseType()->getByteLen();
        if (indexes[0]->isConst()) {
            offset *= static_cast<IR::ConstantInt*>(indexes[0])->getConstValue();
            auto li = new ImmInst(ImmInst::INST_LI, offset);
            _currentBasicBlock->addInstruction(li);
            offsetReg = li->getDest();
        } else {
            if (offset == 4) {
                auto slli = new BinaryInst(BinaryInst::INST_SLLI, _value2reg[indexes[0]], 2);
                _currentBasicBlock->addInstruction(slli);
                offsetReg = slli->getDest();
            } else {
                auto li = new ImmInst(ImmInst::INST_LI, offset);
                _currentBasicBlock->addInstruction(li);
                auto mul = new BinaryInst(BinaryInst::INST_MUL, _value2reg[indexes[0]], li->getDest());
                _currentBasicBlock->addInstruction(mul);
                offsetReg = mul->getDest();
            }
        }
    } else {
        if (indexes[1]->isConst()) {
            int offset = 4 * static_cast<IR::ConstantInt*>(indexes[1])->getConstValue();
            auto li = new ImmInst(ImmInst::INST_LI, offset);
            _currentBasicBlock->addInstruction(li);
            offsetReg = li->getDest();
        } else {
            /// FIXME: the imm should be 3 when base type if array of pointer
            auto slli = new BinaryInst(BinaryInst::INST_SLLI, _value2reg[indexes[1]], 2);
            _currentBasicBlock->addInstruction(slli);
            offsetReg = slli->getDest();
        }
    }
    auto add = new BinaryInst(BinaryInst::INST_ADD, ptr, offsetReg);
    _currentBasicBlock->addInstruction(add);
    _value2reg[inst->getResult()] = add->getDest();
}

void CodeGenerator::emitBitCastInst(IR::BitCastInst* inst) {
    Register* ptr = getRegFromValue(inst->getPtr());
    if (ptr == Register::S0) {
        int offset = _value2offset[inst->getPtr()];
        auto tmp = processIfImmOutOfRange(Register::S0, offset);
        auto getPtr = new BinaryInst(BinaryInst::INST_ADDI, tmp, offset);
        _currentBasicBlock->addInstruction(getPtr);
        _value2reg[inst->getResult()] = getPtr->getDest();
    } else {
        _value2reg[inst->getResult()] = ptr;
    }
}

void CodeGenerator::emitRetInst(IR::ReturnInst* inst) {
    if (inst->getRetValue()) {
        auto retValue = getRegFromValue(inst->getRetValue());
        if (inst->getRetValue()->getType() == IR::Type::getInt32Ty()) {
            _currentBasicBlock->addInstruction(new UnaryInst(UnaryInst::INST_MV, Register::IntArgReg[0], retValue));
        } else {
            _currentBasicBlock->addInstruction(
                new UnaryInst(UnaryInst::INST_FMV_S, Register::FloatArgReg[0], retValue));
        }
    }
    _currentBasicBlock->addInstruction(new JumpInst(_retBB));
}

void CodeGenerator::emitUnaryInst(IR::UnaryInst* inst) {
    Register* src1 = getRegFromValue(inst->getOperand());
    Instruction* unaryInst = nullptr;
    switch (inst->getInstType()) {
        case IR::UnaryInst::INST_LOAD: {
            int offset = inst->getOperand()->isGlobal() ? 0 : _value2offset[inst->getOperand()];
            src1 = processIfImmOutOfRange(src1, offset);
            if (inst->getResult()->getType()->isPointerType()) {
                unaryInst = new LoadInst(LoadInst::INST_LD, src1, offset);
            } else {
                unaryInst = new LoadInst(
                    inst->getResult()->getType() == IR::Type::getInt32Ty() ? LoadInst::INST_LW : LoadInst::INST_FLW,
                    src1, offset);
            }

            break;
        }
        case IR::UnaryInst::INST_ITOF: {
            // optimize covert time when the value must be 0 or 1
            for (auto preInst : _currentBasicBlock->getInstructionList()) {
                if (preInst->getDest() == src1 && preInst->getClassId() == ID_BINARY_INST &&
                    (preInst->getInstType() == BinaryInst::INST_FLT_S ||
                     preInst->getInstType() == BinaryInst::INST_FLE_S ||
                     preInst->getInstType() == BinaryInst::INST_FEQ_S)) {
                    auto oneBB = new BasicBlock();
                    auto zeroBB = new BasicBlock();
                    auto afterBB = new BasicBlock();
                    _currentFunction->addBasicBlock(oneBB);
                    _currentFunction->addBasicBlock(zeroBB);
                    _currentFunction->addBasicBlock(afterBB);

                    auto beq = new CondJumpInst(CondJumpInst::INST_BEQ, src1, Register::Zero, zeroBB);
                    _currentBasicBlock->addInstruction(beq);
                    _currentBasicBlock->addInstruction(new JumpInst(oneBB));

                    _currentBasicBlock = oneBB;
                    Register* dest = loadConstFloat(1);
                    _currentBasicBlock->addInstruction(new JumpInst(afterBB));

                    _currentBasicBlock = zeroBB;
                    _currentBasicBlock->addInstruction(new UnaryInst(UnaryInst::INST_FMV_W_X, dest, Register::Zero));
                    _currentBasicBlock->addInstruction(new JumpInst(afterBB));
                    _currentBasicBlock = afterBB;

                    _value2reg[inst->getResult()] = dest;
                    return;
                }
            }
            unaryInst = new UnaryInst(UnaryInst::INST_FCVT_S_W, src1);
            break;
        }
        case IR::UnaryInst::INST_FTOI:
            unaryInst = new UnaryInst(UnaryInst::INST_FCVT_W_S, src1);
            break;
        default:
            assert(0 && "shouldn't reach here");
            break;
    }
    _currentBasicBlock->addInstruction(unaryInst);
    _value2reg[inst->getResult()] = unaryInst->getDest();
}

void CodeGenerator::emitBinaryInst(IR::BinaryInst* inst) {
    auto dest = inst->getResult();
    auto operand1 = inst->getOperand1();
    auto operand2 = inst->getOperand2();
    if (inst->isIntInst()) {
        _value2reg[dest] = emitIntBinaryInst(inst->getInstType(), operand1, operand2);
    } else {
        _value2reg[dest] = emitFloatBinaryInst(inst->getInstType(), operand1, operand2);
    }
}

void CodeGenerator::emitCondJumpInst(IR::CondJumpInst* inst) {
    Register* src1 = getRegFromValue(inst->getOperand1());
    Register* src2 = getRegFromValue(inst->getOperand2());

    int type;
    if (inst->isIntInst()) {
        switch (inst->getInstType()) {
            case IR::CondJumpInst::INST_JEQ:
                type = CondJumpInst::INST_BEQ;
                break;
            case IR::CondJumpInst::INST_JNE:
                type = CondJumpInst::INST_BNE;
                break;
            case IR::CondJumpInst::INST_JLT:
                type = CondJumpInst::INST_BLT;
                break;
            case IR::CondJumpInst::INST_JLE:
                type = CondJumpInst::INST_BGE;
                std::swap(src1, src2);
                break;
            case IR::CondJumpInst::INST_JGT:
                type = CondJumpInst::INST_BLT;
                std::swap(src1, src2);
                break;
            case IR::CondJumpInst::INST_JGE:
                type = CondJumpInst::INST_BGE;
                break;
            default:
                assert(0 && "can't not reach here");
                break;
        }
    } else {
        Instruction* cmpInst;
        type = CondJumpInst::INST_BNE;
        switch (inst->getInstType()) {
            case IR::CondJumpInst::INST_JEQ:
                cmpInst = new BinaryInst(BinaryInst::INST_FEQ_S, src1, src2);
                break;
            case IR::CondJumpInst::INST_JNE:
                cmpInst = new BinaryInst(BinaryInst::INST_FEQ_S, src1, src2);
                type = CondJumpInst::INST_BEQ;
                break;
            case IR::CondJumpInst::INST_JLT:
                cmpInst = new BinaryInst(BinaryInst::INST_FLT_S, src1, src2);
                break;
            case IR::CondJumpInst::INST_JLE:
                cmpInst = new BinaryInst(BinaryInst::INST_FLE_S, src1, src2);
                break;
            case IR::CondJumpInst::INST_JGT:
                cmpInst = new BinaryInst(BinaryInst::INST_FLT_S, src2, src1);
                break;
            case IR::CondJumpInst::INST_JGE:
                cmpInst = new BinaryInst(BinaryInst::INST_FLE_S, src2, src1);
                break;
            default:
                assert(0 && "can't not reach here");
                break;
        }
        _currentBasicBlock->addInstruction(cmpInst);
        src1 = cmpInst->getDest();
        src2 = loadConstInt(0);
    }
    /// FIXME:Temporarily reslove the problem of relocation failure, need to modify
    auto newBB = new BasicBlock();
    _currentFunction->addBasicBlock(newBB);
    newBB->addInstruction(new JumpInst(_atomBB2asmBB[inst->getTureBB()]));
    _currentBasicBlock->addInstruction(new CondJumpInst(type, src1, src2, newBB));
    _currentBasicBlock->addInstruction(new JumpInst(_atomBB2asmBB[inst->getFalseBB()]));
}

Register* CodeGenerator::emitIntBinaryInst(int instType, IR::Value* operand1, IR::Value* operand2) {
    Register* src1;
    Register* src2 = nullptr;
    int imm;
    bool needXor = false;
    if (operand1->isConst()) {
        imm = static_cast<IR::ConstantInt*>(operand1)->getConstValue();
        switch (instType) {
            case IR::BinaryInst::INST_SUB:
            case IR::BinaryInst::INST_MUL:
            case IR::BinaryInst::INST_DIV:
            case IR::BinaryInst::INST_MOD:
            case IR::BinaryInst::INST_LT: {
                src1 = loadConstInt(imm);
                src2 = getRegFromValue(operand2);
                break;
            }
            case IR::BinaryInst::INST_LE: {
                src1 = loadConstInt(imm - 1);
                src2 = getRegFromValue(operand2);
                break;
            }
            case IR::BinaryInst::INST_GT:
                src1 = getRegFromValue(operand2);
                break;
            case IR::BinaryInst::INST_GE:
                imm += 1;
                src1 = getRegFromValue(operand2);
                break;
            case IR::BinaryInst::INST_EQ:
            case IR::BinaryInst::INST_NE:
                imm = -imm;
                src1 = getRegFromValue(operand2);
                break;
            default:
                src1 = getRegFromValue(operand2);
                break;
        }
    } else if (operand2->isConst()) {
        imm = static_cast<IR::ConstantInt*>(operand2)->getConstValue();
        switch (instType) {
            case IR::BinaryInst::INST_SUB:
                imm = -imm;
                src1 = getRegFromValue(operand1);
                break;
            case IR::BinaryInst::INST_MUL:
            case IR::BinaryInst::INST_DIV:
            case IR::BinaryInst::INST_MOD:
                src1 = getRegFromValue(operand1);
                src2 = loadConstInt(imm);
                break;
            case IR::BinaryInst::INST_LE:
                imm += 1;
                src1 = getRegFromValue(operand1);
                break;
            case IR::BinaryInst::INST_GT:
                src1 = loadConstInt(imm);
                src2 = getRegFromValue(operand1);
                break;
            case IR::BinaryInst::INST_GE:
                src1 = loadConstInt(imm - 1);
                src2 = getRegFromValue(operand1);
                break;
            case IR::BinaryInst::INST_EQ:
            case IR::BinaryInst::INST_NE:
                imm = -imm;
                src1 = getRegFromValue(operand1);
                break;
            default:
                src1 = getRegFromValue(operand1);
                break;
        }
    } else {
        switch (instType) {
            case IR::BinaryInst::INST_LE:
                needXor = true;
                src1 = getRegFromValue(operand2);
                src2 = getRegFromValue(operand1);
                break;
            case IR::BinaryInst::INST_GT:
                src1 = getRegFromValue(operand2);
                src2 = getRegFromValue(operand1);
                break;
            case IR::BinaryInst::INST_GE:
                needXor = true;
            default:
                src1 = getRegFromValue(operand1);
                src2 = getRegFromValue(operand2);
                break;
        }
    }

    Instruction* binaryInst;
    switch (instType) {
        case IR::BinaryInst::INST_ADD:
            if (src2) {
                binaryInst = new BinaryInst(BinaryInst::INST_ADDW, src1, src2);
            } else {
                src1 = processIfImmOutOfRange(src1, imm);
                binaryInst = new BinaryInst(BinaryInst::INST_ADDIW, src1, imm);
            }
            break;
        case IR::BinaryInst::INST_SUB:
            if (src2) {
                binaryInst = new BinaryInst(BinaryInst::INST_SUBW, src1, src2);
            } else {
                src1 = processIfImmOutOfRange(src1, imm);
                binaryInst = new BinaryInst(BinaryInst::INST_ADDIW, src1, imm);
            }
            break;
        case IR::BinaryInst::INST_MUL:
            binaryInst = new BinaryInst(BinaryInst::INST_MULW, src1, src2);
            break;
        case IR::BinaryInst::INST_DIV:
            binaryInst = new BinaryInst(BinaryInst::INST_DIVW, src1, src2);
            break;
        case IR::BinaryInst::INST_MOD:
            binaryInst = new BinaryInst(BinaryInst::INST_REMW, src1, src2);
            break;
        case IR::BinaryInst::INST_LT:
        case IR::BinaryInst::INST_LE:
        case IR::BinaryInst::INST_GT:
        case IR::BinaryInst::INST_GE:
            if (src2) {
                binaryInst = new BinaryInst(BinaryInst::INST_SLT, src1, src2);
            } else {
                if (imm < -2048 || imm > 2047) {
                    src2 = loadConstInt(imm);
                    binaryInst = new BinaryInst(BinaryInst::INST_SLT, src1, src2);
                } else {
                    binaryInst = new BinaryInst(BinaryInst::INST_SLTI, src1, imm);
                }
            }
            break;
        case IR::BinaryInst::INST_EQ:
        case IR::BinaryInst::INST_NE:
            if (src2) {
                binaryInst = new BinaryInst(BinaryInst::INST_XOR, src1, src2);
            } else {
                src1 = processIfImmOutOfRange(src1, imm);
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
        // revert the result
        auto xorInst = new BinaryInst(BinaryInst::INST_XORI, binaryInst->getDest(), 1);
        _currentBasicBlock->addInstruction(xorInst);
        dest = xorInst->getDest();
    }
    if (instType == IR::BinaryInst::INST_EQ) {
        auto seqz = new UnaryInst(UnaryInst::INST_SEQZ, binaryInst->getDest());
        _currentBasicBlock->addInstruction(seqz);
        dest = seqz->getDest();
    } else if (instType == IR::BinaryInst::INST_NE) {
        auto snez = new UnaryInst(UnaryInst::INST_SNEZ, binaryInst->getDest());
        _currentBasicBlock->addInstruction(snez);
        dest = snez->getDest();
    }

    return dest;
}

Register* CodeGenerator::emitFloatBinaryInst(int instType, IR::Value* operand1, IR::Value* operand2) {
    Register* src1 = getRegFromValue(operand1);
    Register* src2 = getRegFromValue(operand2);
    Instruction* binaryInst;
    switch (instType) {
        case IR::BinaryInst::INST_ADD:
            binaryInst = new BinaryInst(BinaryInst::INST_FADD_S, src1, src2);
            break;
        case IR::BinaryInst::INST_SUB:
            binaryInst = new BinaryInst(BinaryInst::INST_FSUB_S, src1, src2);
            break;
        case IR::BinaryInst::INST_MUL:
            binaryInst = new BinaryInst(BinaryInst::INST_FMUL_S, src1, src2);
            break;
        case IR::BinaryInst::INST_DIV:
            binaryInst = new BinaryInst(BinaryInst::INST_FDIV_S, src1, src2);
            break;
        case IR::BinaryInst::INST_LT:
            binaryInst = new BinaryInst(BinaryInst::INST_FLT_S, src1, src2);
            break;
        case IR::BinaryInst::INST_LE:
            binaryInst = new BinaryInst(BinaryInst::INST_FLE_S, src1, src2);
            break;
        case IR::BinaryInst::INST_GT:
            std::swap(src1, src2);
            binaryInst = new BinaryInst(BinaryInst::INST_FLT_S, src1, src2);
            break;
        case IR::BinaryInst::INST_GE:
            std::swap(src1, src2);
            binaryInst = new BinaryInst(BinaryInst::INST_FLE_S, src1, src2);
            break;
        case IR::BinaryInst::INST_EQ:
        case IR::BinaryInst::INST_NE:
            binaryInst = new BinaryInst(BinaryInst::INST_FEQ_S, src1, src2);
            break;
        default:
            assert(0 && "unsupported yet");
            break;
    }
    _currentBasicBlock->addInstruction(binaryInst);
    if (instType == IR::BinaryInst::INST_NE) {
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

Register* CodeGenerator::loadConst(IR::Constant* value) {
    if (value->isInt()) {
        return loadConstInt(static_cast<IR::ConstantInt*>(value)->getConstValue());
    } else {
        return loadConstFloat(static_cast<IR::ConstantFloat*>(value)->getConstValue());
    }
}

Register* CodeGenerator::getRegFromValue(IR::Value* value) {
    if (value->isConst()) {
        return loadConst((IR::Constant*)value);
    }

    if (value->isGlobal()) {
        auto la = new LoadGlobalAddrInst(value->getName());
        _currentBasicBlock->addInstruction(la);
        return la->getDest();
    }

    return _value2reg[value];
}

Register* CodeGenerator::processIfImmOutOfRange(Register* src, int& offset) {
    if (offset < -2048 || offset > 2047) {
        int hi20 = (unsigned)offset >> 12;
        int lo12 = offset & 0xfff;
        if (lo12 > 2047) {
            lo12 -= 4096;
            hi20 += 1;
        }
        auto lui = new ImmInst(ImmInst::INST_LUI, hi20);
        _currentBasicBlock->addInstruction(lui);
        auto add = new BinaryInst(BinaryInst::INST_ADD, src, lui->getDest());
        _currentBasicBlock->addInstruction(add);
        offset = lo12;
        return add->getDest();
    }
    return src;
}

}  // namespace RISCV
}  // namespace ATC
