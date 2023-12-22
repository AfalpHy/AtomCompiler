#pragma once
#include <list>
#include <vector>

#include "Instruction.h"

namespace ATC {
namespace RISCV {

class BasicBlock {
public:
    BasicBlock() : _name(".LBB" + std::to_string(Index++)) {}

    const std::string &getName() { return _name; }

    void addInstruction(Instruction *inst);
    void addPredecessor(BasicBlock *bb) { _predecessors.push_back(bb); }
    void addSuccessor(BasicBlock *bb) { _successors.push_back(bb); }
    void setAlive(const std::set<Register *> &alive) { _alives = alive; }
    void setAnalyzed(bool b) { _analyzed = b; }

    const std::list<Instruction *> &getInstructionList() { return _instructions; }
    const std::vector<BasicBlock *> &getPredecessors() { return _predecessors; }
    const std::vector<BasicBlock *> &getSuccessors() { return _successors; }
    const std::set<Register *> &getAlives() { return _alives; }
    bool isAnalyzed() { return _analyzed; }

    std::string toString() {
        std::string str;
        if (!_name.empty()) {
            str.append(_name + ":\n");
        }
        for (auto inst : _instructions) {
            str.append("\t").append(inst->toString()).append("\n");
        }
        return str;
    }

private:
    static int Index;
    std::string _name;
    std::list<Instruction *> _instructions;
    std::vector<BasicBlock *> _predecessors;
    std::vector<BasicBlock *> _successors;
    std::set<Register *> _alives;
    bool _analyzed = false;
};

}  // namespace RISCV
}  // namespace ATC
