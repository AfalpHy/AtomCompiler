#include <assert.h>

#include <iostream>

#include "atomIR/Function.h"

namespace ATC {
namespace AtomIR {

void Value::setBelong(Function* function) {
    _belong = function;
    _belong->insertName(this);
}

ConstantInt* ConstantInt::get(int value) {
    static std::unordered_map<int, ConstantInt*> num2Value;
    if (num2Value.find(value) != num2Value.end()) {
        return num2Value[value];
    }
    ConstantInt* ret = new ConstantInt(value);
    num2Value.insert({value, ret});
    return ret;
}

ConstantFloat* ConstantFloat::get(float value) {
    static std::unordered_map<float, ConstantFloat*> num2Value;
    if (num2Value.find(value) != num2Value.end()) {
        return num2Value[value];
    }
    ConstantFloat* ret = new ConstantFloat(value);
    num2Value.insert({value, ret});
    return ret;
}

std::string Value::toString() { return _type->toString() + " " + getValueStr(); }

std::string ArrayValue::toString() {
    assert(_type->isArrayType() && "should be array type");
    ArrayType* type = (ArrayType*)_type;
    std::string str = _type->toString();
    Type* baseType = type->getBaseType();
    str.append(" {");
    for (auto& item : _elements) {
        if (item.second.empty()) {
            str.append(std::to_string(item.first).append(" x ")).append(baseType->toString());
            if (baseType == Type::getInt32Ty()) {
                str.append(" 0, ");
            } else {
                str.append(" 0.000000, ");
            }
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

std::string GloabalVariable::getValueStr() { return "@" + _name; }

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
