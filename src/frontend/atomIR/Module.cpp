#include "atomIR/Module.h"

#include <iostream>

#include "atomIR/Function.h"
#include "atomIR/Instruction.h"
namespace ATC {
namespace AtomIR {

void Module::addGlobalVariable(GloabalVariable* var) { _globalVariables.insert({var->getName(), var}); }

void Module::dump() {
    for (auto item : _globalVariables) {
        item.second->dump();
    }

    std::cout << std::endl;

    for (auto item : _functions) {
        item.second->dump();
    }
}
}  // namespace AtomIR
}  // namespace ATC