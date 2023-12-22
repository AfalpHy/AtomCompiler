#pragma once

#include "Function.h"

namespace ATC {

namespace RISCV {

class RegAllocator {
public:
    RegAllocator(Function* function, bool b) : _theFunction(function), _useGraphColoring(b) {}

    void run();

private:
    void buildInterference();
    void coloring();
    void spill();
    void coalescing();

    void analyzeRegAlive(BasicBlock* bb);

private:
    Function* _theFunction;
    bool _useGraphColoring;
};
}  // namespace RISCV
}  // namespace ATC