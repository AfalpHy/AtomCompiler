#include <iostream>

#include "atomIR/Function.h"
#include "atomIR/Instruction.h"

namespace ATC {
namespace AtomIR {

std::string Value::toString() { std::string str = _type->toString() + " " + getValueStr(); }

std::string Value::getValueStr() { return _belong->getUniqueNameInFunction(this); }

std::string ConstantInt::getValueStr() { return std::to_string((int)_constValue); }

std::string ConstantFloat::getValueStr() { return std::to_string(_constValue); }

std::string GloabalVariable::getValueStr() { return _name; }

void Value::dump() { std::cout << _defined->toString() << std::endl; }

void ConstantInt::dump() { std::cout << toString() << std::endl; }

void ConstantFloat::dump() { std::cout << toString() << std::endl; }

/// FIXME: add initialize expression
void GloabalVariable::dump() { std::cout << toString() << std::endl; }

}  // namespace AtomIR
}  // namespace ATC
