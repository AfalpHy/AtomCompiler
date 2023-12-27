#include "riscv/Function.h"

namespace ATC {
namespace RISCV {
void Function::dump() {
    for (auto bb : _basicBlocks) {
        bb->dump();
    }
}
}  // namespace RISCV
}  // namespace ATC
