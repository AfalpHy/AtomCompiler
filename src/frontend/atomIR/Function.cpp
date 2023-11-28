#include <iostream>

#include "atomIR/Instruction.h"
namespace ATC {

namespace AtomIR {
std::string Function::getUniqueNameInFunction(const std::string& name) {
    std::string ret;
    if (name.empty()) {
        ret = std::string("%") + std::to_string(_valueIndex++);
    } else {
        ret = std::string("%") + name;
    }

    while (_nameSet.find(ret) != _nameSet.end()) {
        if (name.empty()) {
            ret = std::string("%") + std::to_string(_valueIndex++);
        } else {
            ret = std::string("%") + name + std::to_string(_valueIndex++);
        }
    }
    _nameSet.insert(ret);
    return ret;
}

void Function::updateNameIfNeed() {
    if (!_needUpdateName) {
        return;
    }
    for (auto param : _params) {
        if (param->isConst()) {
            continue;
        }
        param->setCurrentName(getUniqueNameInFunction(param->getOriginName()));
    }
    for (auto bb : _basicBlocks) {
        bb->setCurrentName(getUniqueNameInFunction(bb->getOriginName()));
        for (auto inst : bb->getInstructionList()) {
            if (Value* result = inst->getResult()) {
                if (result->isConst()) {
                    continue;
                }
                result->setCurrentName(getUniqueNameInFunction(result->getOriginName()));
            }
        }
    }
    _needUpdateName = false;
    _nameSet.clear();
    _valueIndex = 0;
}

void Function::dump() {
    updateNameIfNeed();
    std::cout << _functionType._ret->toString() << " " << _name << "(";
    std::string paramsStr;
    for (auto param : _params) {
        paramsStr = param->getType()->toString() + " " + param->getCurrentName() + ",";
    }
    if (!paramsStr.empty()) {
        paramsStr.pop_back();
    }
    std::cout << paramsStr << ") {" << std::endl;
    for (auto bb : _basicBlocks) {
        std::cout << bb->getCurrentName() << ":" << std::endl;
        for (auto inst : bb->getInstructionList()) {
            std::cout << "  " << inst->toString() << std::endl;
        }
    }
    std::cout << paramsStr << "}" << std::endl;
}
}  // namespace AtomIR
}  // namespace ATC