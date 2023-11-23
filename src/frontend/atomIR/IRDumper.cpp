#include "atomIR/IRDumper.h"

#include <iostream>

#include "atomIR/BasicBlock.h"
#include "atomIR/Function.h"
#include "atomIR/Instruction.h"
namespace ATC {
namespace AtomIR {
void IRDumper::dump(Module* module) {
    for (auto item : module->getFunctions()) {
        for (auto bb : item.second->getBasicBlocks()) {
            for (auto inst : bb->getInstructionList()) {
                std::cout << inst->toString() << std::endl;
            }
        }
    }
}
}  // namespace AtomIR
}  // namespace ATC