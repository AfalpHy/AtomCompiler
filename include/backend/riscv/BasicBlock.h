#pragma once
#include <list>
#include <vector>

#include "Instruction.h"

namespace ATC {
namespace RISCV {

class Function;

class BasicBlock {
public:
    BasicBlock(const std::string &name = "");

    const std::string &getName() { return _name; }

    void setIsEntry() { _entry = true; }
    void addInstruction(Instruction *inst);
    void addPredecessor(BasicBlock *bb) { _predecessors.push_back(bb); }
    void addSuccessor(BasicBlock *bb) { _successors.push_back(bb); }
    void setAlives(const std::set<Register *> &alives) { _alives = alives; }

    const std::list<Instruction *> &getInstructionList() { return _instructions; }
    std::list<Instruction *> &getMutableInstructionList() { return _instructions; }
    const std::vector<BasicBlock *> &getPredecessors() { return _predecessors; }
    const std::vector<BasicBlock *> &getSuccessors() { return _successors; }
    const std::set<Register *> &getAlives() { return _alives; }

    std::string toString() {
        std::string str;
        if (!_entry) {
            if (!_predecessors.empty()) {
                return "";
            }
            str.append(_name + ":\n");
        }
        for (auto inst : _instructions) {
            str.append("\t").append(inst->toString()).append("\n");
        }
        return str;
    }

    void reset() { _alives.clear(); }

    // for debug
    void dump();

private:
    static int Index;
    bool _entry = false;
    std::string _name;
    std::list<Instruction *> _instructions;
    std::vector<BasicBlock *> _predecessors;
    std::vector<BasicBlock *> _successors;
    std::set<Register *> _alives;
};

}  // namespace RISCV
}  // namespace ATC
