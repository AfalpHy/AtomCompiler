#include "atomIR/Module.h"

namespace ATC {
namespace AtomIR {

void Module::dump(std::ostream& os) {
    for (auto item : _globalVariables) {
        item->dump(os);
    }

    os << std::endl;

    for (auto item : _functions) {
        item->dump(os);
    }
}

}  // namespace AtomIR
}  // namespace ATC