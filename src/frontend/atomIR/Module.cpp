#include "atomIR/Module.h"

#include <iostream>

#include "atomIR/Function.h"
#include "atomIR/Instruction.h"
namespace ATC {
namespace AtomIR {
void Module::addGlobalVariable(Value* var) {
    static int _valueIndex = 0;
    std::string name = var->getOriginName();
    if (name.empty()) {
        name = std::string("@") + std::to_string(_valueIndex++);
    } else {
        name = std::string("@") + name;
    }

    while (_globalVariables.find(name) != _globalVariables.end()) {
        if (name.empty()) {
            name = std::string("@") + std::to_string(_valueIndex++);
        } else {
            name = std::string("@") + name + std::to_string(_valueIndex++);
        }
    }
    var->setCurrentName(name);
    _globalVariables.insert({name, var});
}

void Module::dump() {
    /// TODO:global variable

    for (auto item : _functions) {
        item.second->dump();
    }
}
}  // namespace AtomIR
}  // namespace ATC