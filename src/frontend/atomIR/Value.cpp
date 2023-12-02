#include <iostream>

#include "assert.h"
#include "atomIR/Function.h"
#include "atomIR/Instruction.h"

namespace ATC {
namespace AtomIR {

void Value::setBelong(Function* function) {
    _belong = function;
    _belong->insertName(this);
}

std::string Value::toString() { return _type->toString() + " " + getValueStr(); }

std::string ArrayValue::toString() {
    assert(_type->isArrayType() && "should be array type");
    ArrayType* type = (ArrayType*)_type;
    std::string str = _type->toString();

    str.append(" {");
    for (auto& item : _elements) {
        if (item.second.empty()) {
            str.append(std::to_string(item.first).append(" x ")).append(type->getBaseType()->toString()).append(" 0, ");
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

std::string ConstantInt::getValueStr() { return std::to_string(_constValue); }

std::string ConstantFloat::getValueStr() { return std::to_string(_constValue); }

std::string GloabalVariable::getValueStr() { return _name; }

void Value::dump() {
    if (!_defined) {
        std::cout << toString() << std::endl;
    } else {
        std::cout << _defined->toString() << std::endl;
    }
}

void GloabalVariable::dump() {
    std::string str = getValueStr();
    if (_init) {
        str.append(" = ").append(_init->toString());
    }
    std::cout << str << std::endl;
}

}  // namespace AtomIR
}  // namespace ATC
