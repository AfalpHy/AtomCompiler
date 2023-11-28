#include <iostream>

#include "atomIR/Function.h"

namespace ATC {
namespace AtomIR {
Value::Value(Type* type, float value) : _type(type), _isConst(true), _constValue(value) {
    if (type->getTypeEnum() == INT32_TY) {
        _currentName = std::to_string((int)value);
    } else {
        _currentName = std::to_string(value);
    }
}

void Value::dump() {
    _belongFunction->updateNameIfNeed();
    std::cout << _type->toString() << " " << _currentName << std::endl;
}
}  // namespace AtomIR
}  // namespace ATC
