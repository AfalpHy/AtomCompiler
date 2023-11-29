#include <assert.h>

#include <iostream>

#include "atomIR/Instruction.h"
namespace ATC {

namespace AtomIR {

std::string Function::getUniqueNameInFunction(void* ptr) {
    updateNameIfNeed();
    auto find = _nameMap.find(ptr);
    assert(find != _nameMap.end() && "should get a unique name");
    return find->second;
}

void Function::insertName(Value* value) {
    std::string name = value->getName();
    std::string uniqueName;
    if (name.empty()) {
        uniqueName = std::string("%") + std::to_string(_valueIndex++);
    } else {
        uniqueName = std::string("%") + name;
    }

    while (_nameSet.find(uniqueName) != _nameSet.end()) {
        if (name.empty()) {
            uniqueName = std::string("%") + std::to_string(_valueIndex++);
        } else {
            uniqueName = std::string("%") + name + std::to_string(_valueIndex++);
        }
    }
    _nameSet.insert(uniqueName);
    _nameMap.insert({value, uniqueName});
}

void Function::insertName(BasicBlock* bb) {
    std::string name = bb->getName();
    std::string uniqueName;
    if (name.empty()) {
        uniqueName = std::string("%") + std::to_string(_valueIndex++);
    } else {
        uniqueName = std::string("%") + name;
    }

    while (_nameSet.find(uniqueName) != _nameSet.end()) {
        if (name.empty()) {
            uniqueName = std::string("%") + std::to_string(_valueIndex++);
        } else {
            uniqueName = std::string("%") + name + std::to_string(_valueIndex++);
        }
    }
    _nameSet.insert(uniqueName);
    _nameMap.insert({bb, uniqueName});
}

void Function::updateNameIfNeed() {
    if (!_needUpdateName) {
        return;
    }
    _needUpdateName = false;
    _valueIndex = 0;
    _nameSet.clear();
    _nameMap.clear();
    for (auto param : _params) {
        if (param->isConst()) {
            continue;
        }
        insertName(param);
    }
    for (auto bb : _basicBlocks) {
        insertName(bb);
        for (auto inst : bb->getInstructionList()) {
            if (Value* result = inst->getResult()) {
                if (result->isConst()) {
                    continue;
                }
                insertName(result);
            }
        }
    }
}

void Function::dump() {
    updateNameIfNeed();
    std::cout << _functionType._ret->toString() << " " << _name << "(";
    std::string paramsStr;
    for (auto param : _params) {
        paramsStr = param->getType()->toString() + " " + param->getName() + ",";
    }
    if (!paramsStr.empty()) {
        paramsStr.pop_back();
    }
    std::cout << paramsStr << ") {" << std::endl;
    for (auto bb : _basicBlocks) {
        std::cout << getUniqueNameInFunction(bb) << ":" << std::endl;
        for (auto inst : bb->getInstructionList()) {
            std::cout << "  " << inst->toString() << std::endl;
        }
    }
    std::cout << paramsStr << "}" << std::endl;
}
}  // namespace AtomIR
}  // namespace ATC