#include "atomIR/Function.h"

namespace ATC {
namespace AtomIR {

std::string BasicBlock::getBBStr() { return _parent->getUniqueNameInFunction(this); }

}  // namespace AtomIR
}  // namespace ATC