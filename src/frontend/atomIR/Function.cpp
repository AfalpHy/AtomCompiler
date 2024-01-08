#include <assert.h>

#include <iostream>
#include <sstream>

#include "atomIR/Module.h"

namespace ATC {

namespace AtomIR {

Function::Function(Module* parent, const FunctionType& functionType, const std::string& name)
    : _parent(parent), _functionType(functionType), _name(name) {
    for (auto paramType : functionType._params) {
        Value* param = new Value(paramType, "");
        param->setBelong(this);
        _params.push_back(param);
    }
    parent->addFunction(this);
}

std::string Function::getUniqueNameInFunction(void* ptr) { return _nameMap[ptr]; }

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

void Function::updateName() {
    _valueIndex = 0;
    _nameSet.clear();
    _nameMap.clear();
    for (auto param : _params) {
        insertName(param);
    }
    for (auto bb : _basicBlocks) {
        insertName(bb);
        for (auto inst : bb->getInstructionList()) {
            if (Value* result = inst->getResult()) {
                insertName(result);
            }
        }
    }
}

std::string Function::toString() {
    std::stringstream ss;
    ss << "define " << _functionType._ret->toString() << " " << _name << "(";
    std::string paramsStr;
    for (auto param : _params) {
        paramsStr.append(param->toString() + ", ");
    }
    if (!paramsStr.empty()) {
        paramsStr.pop_back();
        paramsStr.pop_back();
    }
    ss << paramsStr << ") {" << std::endl;
    for (auto bb : _basicBlocks) {
        ss << bb->getBBStr() << ":" << std::endl;
        for (auto inst : bb->getInstructionList()) {
            ss << "  " << inst->toString() << std::endl;
        }
        if (bb != _basicBlocks.back()) {
            ss << std::endl;
        }
    }
    ss << "}" << std::endl;
    return ss.str();
}

void Function::dump() { std::cout << toString() << std::endl; }

}  // namespace AtomIR
}  // namespace ATC