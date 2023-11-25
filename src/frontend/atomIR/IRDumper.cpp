#include "atomIR/IRDumper.h"

#include <iostream>

#include "atomIR/BasicBlock.h"
#include "atomIR/Function.h"
#include "atomIR/Instruction.h"
namespace ATC {
namespace AtomIR {
void IRDumper::dump(Module* module) {
    for (auto item : module->getFunctions()) {
        Function* func = item.second;
        for (auto param : func->getParams()) {
            param->setName(func->getUniqueValueName(param->getName()));
        }
        for (auto bb : func->getBasicBlocks()) {
            for (auto inst : bb->getInstructionList()) {
                if (Value* result = inst->getResult()) {
                    result->setName(func->getUniqueValueName(result->getName()));
                }
                std::cout << inst->toString() << std::endl;
            }
        }
    }
}
}  // namespace AtomIR
}  // namespace ATC