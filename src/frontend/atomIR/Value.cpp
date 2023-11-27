#include <iostream>

#include "atomIR/Function.h"

namespace ATC {
namespace AtomIR {
void Value::dump() {
    _belongFunction->updateNameIfNeed();
    std::cout << _type->toString() << " " << _currentName << std::endl;
}
}  // namespace AtomIR
}  // namespace ATC
