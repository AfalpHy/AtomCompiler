#include "riscv/CodeGenerator.h"

#include <assert.h>

#include "atomIR/Instruction.h"
#include "atomIR/Module.h"
#include "riscv/BasicBlock.h"
#include "riscv/Function.h"

namespace ATC {
namespace RISCV_ARCH {

using std::endl;

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

void CodeGenerator::emitAllocInst(AtomIR::AllocInst* inst) {}

void CodeGenerator::emitStoreInst(AtomIR::StoreInst* inst) {}

void CodeGenerator::emitFunctionCallInst(AtomIR::FunctionCallInst* inst) {}

void CodeGenerator::emitGEPInst(AtomIR::GetElementPtrInst* inst) {}

void CodeGenerator::emitRetInst(AtomIR::ReturnInst* inst) {}

void CodeGenerator::emitUnaryInst(AtomIR::UnaryInst* inst) {}

void CodeGenerator::emitBinaryInst(AtomIR::BinaryInst* inst) {}

void CodeGenerator::emitCondJumpInst(AtomIR::CondJumpInst* inst) {}

}  // namespace RISCV_ARCH
}  // namespace ATC
