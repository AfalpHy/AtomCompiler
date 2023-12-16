#include "atomIR/Module.h"

#include <iostream>

namespace ATC {
namespace AtomIR {

void Module::addGlobalVariable(GloabalVariable* var) { _globalVariables.push_back(var); }

void Module::dump() {
    for (auto item : _globalVariables) {
        item->dump();
    }

    std::cout << std::endl;

    for (auto item : _functions) {
        item->dump();
    }
}
}  // namespace AtomIR
}  // namespace ATC