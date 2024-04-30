#include "atomIR/Module.h"

#include <fstream>
#include <sstream>

namespace ATC {
namespace AtomIR {

std::string Module::toString() {
    std::stringstream ss;
    for (auto item : _globalVariables) {
        ss << item->getValueStr() << " = " << item->getInitialValue()->toString() << std::endl;
    }

    ss << std::endl;

    for (auto item : _functions) {
        ss << item->toString() << std::endl;
    }
    return ss.str();
}

void Module::dump() { std::cout << toString() << std::endl; }

void Module::print(const std::string& filePath) {
    std::ofstream out(filePath, std::ios::trunc);
    out << toString() << std::endl;
}
}  // namespace AtomIR
}  // namespace ATC