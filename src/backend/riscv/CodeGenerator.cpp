#include "riscv/CodeGenerator.h"

#include <assert.h>

#include "atomIR/Instruction.h"
#include "atomIR/Module.h"
#include "riscv/BasicBlock.h"
#include "riscv/Function.h"

namespace ATC {
namespace RISCV {

using std::endl;

CodeGenerator::CodeGenerator() { _sp = new Register("sp"); }

void CodeGenerator::dump(std::ostream& os) { os << _contend.str() << endl; }

void CodeGenerator::emitModule(AtomIR::Module* module) {
    for (auto& item : module->getGlobalVariables()) {
        emitGlobalVariable(item.second);
    }

    for (auto& item : module->getFunctions()) {
        emitFunction(item.second);
    }
}

void CodeGenerator::emitGlobalVariable(AtomIR::GloabalVariable* var) {
    _contend << "\t.type\t" << var->getName() << ",@object" << endl;
    _contend << "\t.data" << endl;
    _contend << "\tglobal\t" << var->getName() << endl;
    _contend << "\t.p2align\t2" << endl;
    _contend << var->getName() << "." << endl;
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
    _contend << "\t.size\t" << var->getName() << "," << std::to_string(size) << endl << endl;
}

void CodeGenerator::emitFunction(AtomIR::Function* function) {
    _currentFunction = new Function();
    for (auto bb : function->getBasicBlocks()) {
        emitBasicBlock(bb);
    }

    /// TODO: modify the instruct

    /// append completed code into content
    for (auto bb : _currentFunction->getBasicBlocks()) {
        _contend << bb->toString() << endl;
    }
}

void CodeGenerator::emitBasicBlock(AtomIR::BasicBlock* basicBlock) {
    _currentBasicBlock = new BasicBlock(basicBlock->getBBStr());
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
            _currentBasicBlock->addInstruction(new JumpInst(jumpInst->getTargetBB()->getBBStr()));
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
    auto type = static_cast<AtomIR::PointerType*>(inst->getResult()->getType())->getBaseType();
    _offset += type->getByteLen();
    _value2reg[inst->getResult()] = _sp;
}

void CodeGenerator::emitStoreInst(AtomIR::StoreInst* inst) {
    auto value = inst->getValue();
    auto dest = inst->getDest();
    int instType;
    if (inst->isIntInst()) {
        instType = StoreInst::INST_SW;
    } else {
        /// TODO:fsw
    }
    Register* src1;
    if (value->isConst()) {
        AtomIR::Constant* constValue = (AtomIR::Constant*)value;
        if (constValue->isInt()) {
            src1 = loadConstInt(static_cast<AtomIR::ConstantInt*>(constValue)->getConstValue(), "");
            _value2reg[value] = src1;
        } else {
            // src1 = loadConstFloat(static_cast<AtomIR::ConstantFloat*>(constValue)->getConstValue(),"");
        }
    } else {
        src1 = _value2reg[inst->getValue()];
    }

    if (dest->isGlobal()) {
        auto la = new LoadGlobalAddrInst(dest->getName());
        _currentBasicBlock->addInstruction(la);
        _currentBasicBlock->addInstruction(new StoreInst(instType, src1, la->getDest(), 0));
    } else {
        _currentBasicBlock->addInstruction(new StoreInst(instType, src1, _sp, _value2offset[inst->getDest()]));
    }
}

void CodeGenerator::emitFunctionCallInst(AtomIR::FunctionCallInst* inst) {
    _currentBasicBlock->addInstruction(new FunctionCallInst(inst->getFuncName()));
    if (inst->getResult()) {
        _value2reg[inst->getResult()] = new Register("");
    }
}

void CodeGenerator::emitGEPInst(AtomIR::GetElementPtrInst* inst) {}

void CodeGenerator::emitRetInst(AtomIR::ReturnInst* inst) { _currentBasicBlock->addInstruction(new ReturnInst()); }

void CodeGenerator::emitUnaryInst(AtomIR::UnaryInst* inst) {
    switch (inst->getInstType()) {
        case AtomIR::UnaryInst::INST_LOAD: {
            auto riscvInst =
                new LoadInst(LoadInst::INST_LW, _value2reg[inst->getOperand()], _value2offset[inst->getOperand()]);
            _value2reg[inst->getResult()] = riscvInst->getDest();
            _currentBasicBlock->addInstruction(riscvInst);
            break;
        }

        case AtomIR::UnaryInst::INST_ITOF:
            break;
        case AtomIR::UnaryInst::INST_FTOI:
            break;
        default:
            assert(0 && "shouldn't reach here");
            break;
    }
}

void CodeGenerator::emitBinaryInst(AtomIR::BinaryInst* inst) {
    auto dest = inst->getResult();
    auto operand1 = inst->getOperand1();
    auto operand2 = inst->getOperand2();

    switch (inst->getInstType()) {
        case AtomIR::BinaryInst::INST_ADD: {
            auto riscvInst = new BinaryInst(BinaryInst::INST_ADD, _value2reg[operand1], _value2reg[operand2]);
            _value2reg[inst->getResult()] = riscvInst->getDest();
            _currentBasicBlock->addInstruction(riscvInst);
            break;
        }
        default:
            break;
    }
}

void CodeGenerator::emitCondJumpInst(AtomIR::CondJumpInst* inst) {}

Register* CodeGenerator::loadConstInt(int value, const std::string& name) {
    auto li = new ImmInst(ImmInst::INST_LI, value, name);
    _currentBasicBlock->addInstruction(li);
    return li->getDest();
}

}  // namespace RISCV
}  // namespace ATC
