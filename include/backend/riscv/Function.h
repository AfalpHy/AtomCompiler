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

    void addNeedAllocReg(Register* reg) { _needAllocRegs.insert(reg); }

    void addNeedPushReg(Register* reg) { _needPushRegs.insert(reg); }

    const std::list<BasicBlock*>& getBasicBlocks() { return _basicBlocks; }

    std::list<BasicBlock*>& getMutableBasicBlocks() { return _basicBlocks; }

    std::set<Register*>& getNeedAllocRegs() { return _needAllocRegs; }

    std::set<Register*>& getNeedPushRegs() { return _needPushRegs; }

    static std::vector<Register*> CallerSavedRegs;
    static std::vector<Register*> CalleeSavedRegs;

    std::string toString();

    // for debug
    void dump();

private:
    std::string _name;
    std::list<BasicBlock*> _basicBlocks;
    std::set<Register*> _needAllocRegs;
    std::set<Register*> _needPushRegs;
};

}  // namespace RISCV
}  // namespace ATC