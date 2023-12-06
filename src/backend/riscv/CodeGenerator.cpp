#include "riscv/CodeGenerator.h"

#include "atomIR/Instruction.h"
#include "atomIR/Module.h"
#include "riscv/BasicBlock.h"
#include "riscv/Function.h"
#include "riscv/Module.h"

namespace ATC {
namespace RISCV_ARCH {

void CodeGenerator::dump(std::ostream& os) {
    if (_currentModule) {
        os << _currentModule->getContent() << std::endl;
    }
}

void CodeGenerator::emitModule(AtomIR::Module* module) {
    _currentModule = new Module();

    for (auto& item : module->getGlobalVariables()) {
        emitGlobalVariable(item.second);
    }

    for (auto& item : module->getFunctions()) {
        emitFunction(item.second);
    }
}

void CodeGenerator::emitGlobalVariable(AtomIR::GloabalVariable* var) {}

void CodeGenerator::emitFunction(AtomIR::Function* function) {
    _currentFunction = new Function();
    for (auto bb : function->getBasicBlocks()) {
        emitBasicBlock(bb);
    }
}

void CodeGenerator::emitBasicBlock(AtomIR::BasicBlock* basicBlock) {
    _currentBasicBlock = new BasicBlock();
    for (auto inst : basicBlock->getInstructionList()) {
    }
}

void CodeGenerator::emitInstruction(AtomIR::Instruction* inst) {
    switch (inst->getClassId()) {
        case AtomIR::ID_ALLOC_INST:
            /* code */
            break;

        default:
            break;
    }
}

}  // namespace RISCV_ARCH
}  // namespace ATC
