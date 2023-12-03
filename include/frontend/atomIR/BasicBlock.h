#pragma once

#include <list>
#include <string>
#include <vector>

namespace ATC {
namespace AtomIR {

class Instruction;
class Function;

class BasicBlock {
public:
    BasicBlock(Function* parent, const std::string& name) : _parent(parent), _name(name) {}

    void addPredecessor(BasicBlock* bb) { _predecessors.push_back(bb); }
    void addSuccessor(BasicBlock* bb) { _successors.push_back(bb); }
    void addInstruction(Instruction* inst);

    Function* getParent() { return _parent; }
    const std::string& getName() { return _name; }
    const std::vector<BasicBlock*>& getPredecessors() { return _predecessors; }
    const std::vector<BasicBlock*>& getSuccessors() { return _successors; }
    const std::list<Instruction*>& getInstructionList() { return _instructions; }

    std::string getBBStr();

private:
    Function* _parent;
    std::string _name;
    std::vector<BasicBlock*> _predecessors;
    std::vector<BasicBlock*> _successors;
    std::list<Instruction*> _instructions;
};

}  // namespace AtomIR
}  // namespace ATC
