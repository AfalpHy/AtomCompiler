#include "atomIR/Module.h"

#include <iostream>

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