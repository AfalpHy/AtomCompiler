#ifndef ATOM_IR_DUMPER_H
#define ATOM_IR_DUMPER_H

#include "Module.h"
namespace ATC {

namespace AtomIR {
class IRDumper {
public:
    void dump(Module *module);
};

}  // namespace AtomIR
}  // namespace ATC
#endif