#ifndef ATOM_BASIC_BLOCK_H
#define ATOM_BASIC_BLOCK_H
#include <list>
#include <string>
#include <vector>

namespace ATC {
namespace AtomIR {

class Instruction;
class Function;

class BasicBlock {
public:
    BasicBlock(Function* parent, const std::string& name) : _parent(parent), _originName(name) {}

    void setCurrentName(const std::string& name) { _currentName = name; }
    void addPredecessor(BasicBlock* bb) { _predecessors.push_back(bb); }
    void addSuccessor(BasicBlock* bb) { _successors.push_back(bb); }
    void addInstruction(Instruction* inst) { _instructions.push_back(inst); }

    Function* getParent() { return _parent; }
    const std::string& getOriginName() { return _currentName; }
    const std::string& getCurrentName() { return _currentName; }
    const std::vector<BasicBlock*>& getPredecessors() { return _predecessors; }
    const std::vector<BasicBlock*>& getSuccessors() { return _successors; }
    const std::list<Instruction*>& getInstructionList() { return _instructions; }

private:
    Function* _parent;
    std::string _originName;
    std::string _currentName;
    std::vector<BasicBlock*> _predecessors;
    std::vector<BasicBlock*> _successors;
    std::list<Instruction*> _instructions;
};

}  // namespace AtomIR
}  // namespace ATC

#endif