#include "riscv/Function.h"

#include <iostream>
#include <sstream>
namespace ATC {
namespace RISCV {

using std::endl;

std::string Function::toString() {
    std::stringstream ss;
    ss << "\t.globl\t" << _name << endl;
    ss << "\t.p2align\t1" << endl;
    ss << "\t.type\t" << _name << ",@function" << endl;
    ss << _name << ":" << endl;
    for (auto bb : _basicBlocks) {
        ss << bb->toString();
    }
    ss << "\t.size\t" << _name << ", .-" << _name << endl << endl;
    return ss.str();
}

void Function::dump() { std::cout << toString() << endl; }

}  // namespace RISCV
}  // namespace ATC
