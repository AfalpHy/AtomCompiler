#include "atomIR/Module.h"

#include <iostream>

#include "atomIR/Function.h"
#include "atomIR/Instruction.h"
namespace ATC {
namespace AtomIR {
void Module::addGlobalVariable(Value* var) {
    static int _valueIndex = 0;
    std::string name = var->getName();
    std::string uniqueName;
    if (name.empty()) {
        uniqueName = std::string("@") + std::to_string(_valueIndex++);
    } else {
        uniqueName = std::string("@") + name;
    }

    while (_globalVariables.find(name) != _globalVariables.end()) {
        if (name.empty()) {
            uniqueName = std::string("@") + std::to_string(_valueIndex++);
        } else {
            uniqueName = std::string("@") + name + std::to_string(_valueIndex++);
        }
    }
    var->setName(uniqueName);
    _globalVariables.insert({uniqueName, var});
}

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