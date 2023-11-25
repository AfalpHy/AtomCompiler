#include "atomIR/Function.h"

namespace ATC {

namespace AtomIR {
std::string Function::getUniqueValueName(const std::string& name) {
    std::string ret = name;
    if (ret.empty()) {
        ret = std::to_string(_valueIndex++);
    }
    while (_valueNameSet.find(ret) != _valueNameSet.end()) {
        ret = name + std::to_string(_valueIndex++);
    }
    _valueNameSet.insert(ret);
    return "%" + ret;
}
}  // namespace AtomIR
}  // namespace ATC