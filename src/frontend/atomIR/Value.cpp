#include <iostream>

#include "atomIR/Function.h"
#include "atomIR/Instruction.h"

namespace ATC {
namespace AtomIR {

std::string Value::toString() { return _type->toString() + " " + getValueStr(); }

std::string ArrayValue::toString() {
    std::string str;

    str.append("{");
    for (auto& item : _elements) {
        if (item.second.empty()) {
            str.append(std::to_string(item.first).append(" x ")).append(_type->toString()).append(", ");
        } else {
            for (auto& element : item.second) {
                str.append(element->toString()).append(", ");
            }
        }
    }
    str.pop_back();
    str.pop_back();
    str.append("}");

    return str;
}

std::string Value::getValueStr() { return _belong->getUniqueNameInFunction(this); }

std::string ConstantInt::getValueStr() { return std::to_string((int)_constValue); }

std::string ConstantFloat::getValueStr() { return std::to_string(_constValue); }

std::string GloabalVariable::getValueStr() { return _name; }

void Value::dump() { std::cout << _defined->toString() << std::endl; }

void ConstantInt::dump() { std::cout << toString() << std::endl; }

void ConstantFloat::dump() { std::cout << toString() << std::endl; }

void ArrayValue::dump() { std::cout << toString() << std::endl; }

void GloabalVariable::dump() {
    std::string str = static_cast<PointerType*>(_type)->getBaseType()->toString() + " " + getValueStr();
    if (_init) {
        str.append(" = ").append(_init->toString());
    }
    std::cout << str << std::endl;
}

}  // namespace AtomIR
}  // namespace ATC
