#pragma once

#include "Function.h"

namespace ATC {

namespace RISCV {

class RegAllocator {
public:
    RegAllocator(Function* function, int& currentOffset, bool b)
        : _theFunction(function), _currentOffset(currentOffset), _useGraphColoring(b) {}

    void run();

private:
    void buildInterference();
    void coalescing();
    bool coloring();
    void spill();

    void reset();

private:
    Function* _theFunction;
    int& _currentOffset;  // for spill reg
    bool _useGraphColoring;
    Register* _needSpill = nullptr;
};
}  // namespace RISCV
}  // namespace ATC