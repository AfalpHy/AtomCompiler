#pragma once

#include <set>
#include <string>

#include "BasicBlock.h"
namespace ATC {

namespace RISCV {

class Function {
public:
    Function(const std::string& name) : _name(name) {}

    void insertFront(BasicBlock* bb) { _basicBlocks.push_front(bb); }

    void addBasicBlock(BasicBlock* bb) { _basicBlocks.push_back(bb); }

    const std::list<BasicBlock*>& getBasicBlocks() { return _basicBlocks; }

    static std::set<Register*> AllRegInFunction;

    static std::vector<Register*> CallerSavedRegs;

    std::string toString();
     
    // for debug
    void dump();

private:
    std::string _name;
    std::list<BasicBlock*> _basicBlocks;
};

}  // namespace RISCV
}  // namespace ATC