#pragma once

#include <list>

#include "Instruction.h"

namespace ATC {
namespace IR {

class Function;

class BasicBlock {
public:
    BasicBlock(Function* parent, const std::string& name);

    void addPredecessor(BasicBlock* bb) { _predecessors.push_back(bb); }
    void addSuccessor(BasicBlock* bb) { _successors.push_back(bb); }
    void addInstruction(Instruction* inst);
    void setAlives(const std::set<Value*>& alives) { _alives = alives; }
    void setHasBr() { _hasBr = true; }

    Function* getParent() { return _parent; }
    const std::string& getName() { return _name; }
    const std::vector<BasicBlock*>& getPredecessors() { return _predecessors; }
    const std::vector<BasicBlock*>& getSuccessors() { return _successors; }
    std::list<Instruction*>& getInstructionList() { return _instructions; }
    const std::set<Value*>& getAlives() { return _alives; }
    bool isHasBr() { return _hasBr; }

    std::string getBBStr();

private:
    Function* _parent;
    std::string _name;
    std::vector<BasicBlock*> _predecessors;
    std::vector<BasicBlock*> _successors;
    std::list<Instruction*> _instructions;
    std::set<Value*> _alives;
    bool _hasBr = false;
};

}  // namespace IR
}  // namespace ATC
