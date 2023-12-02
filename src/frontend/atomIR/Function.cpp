#include <assert.h>

#include <iostream>

#include "atomIR/Instruction.h"
#
namespace ATC {

namespace AtomIR {

Function::Function(Module* parent, const FunctionType& functionType, const std::string& name)
    : _parent(parent), _functionType(functionType), _name(name) {
    for (auto paramType : functionType._params) {
        Value* param = new Value(paramType, "");
        param->setBelong(this);
        _params.push_back(param);
    }
}

std::string Function::getUniqueNameInFunction(void* ptr) {
    updateNameIfNeed();
    auto find = _nameMap.find(ptr);
    assert(find != _nameMap.end() && "should get a unique name");
    return find->second;
}

void Function::insertName(Value* value) { insertName(value, value->getName()); }

void Function::insertName(BasicBlock* bb) { insertName(bb, bb->getName()); }

void Function::insertName(void* ptr, const std::string& name) {
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
    _nameMap.insert({ptr, uniqueName});
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
    std::cout << "define " << _functionType._ret->toString() << " " << _name << "(";
    std::string paramsStr;
    for (auto param : _params) {
        paramsStr.append(param->toString() + ", ");
    }
    if (!paramsStr.empty()) {
        paramsStr.pop_back();
        paramsStr.pop_back();
    }
    std::cout << paramsStr << ") {" << std::endl;
    for (auto bb : _basicBlocks) {
        std::cout << bb->getBBStr() << ":" << std::endl;
        for (auto inst : bb->getInstructionList()) {
            std::cout << "  " << inst->toString() << std::endl;
        }
    }
    std::cout << "}" << std::endl;
}

}  // namespace AtomIR
}  // namespace ATC