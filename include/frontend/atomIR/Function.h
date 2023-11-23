#ifndef ATOM_FUNCTION_H
#define ATOM_FUNCTION_H

#include <unordered_map>

#include "BasicBlock.h"
#include "Value.h"

namespace ATC {
namespace AtomIR {

class Module;

class Function {
public:
    Function(Module* parent, const std::string& name) : _parent(parent), _name(name) {}

    void addParam(Value* param) { _params.push_back(param); }
    void insertBB(BasicBlock* bb) { _basicBlocks.push_back(bb); }

    Module* getParent() { return _parent; }

    const std::string& getName() { return _name; }

    BasicBlock* getEnteryBB() {
        if (_basicBlocks.size() > 0) {
            return _basicBlocks[0];
        }
        return nullptr;
    }

    const std::vector<Value*> getParams() { return _params; }
    const std::vector<BasicBlock*> getBasicBlocks() { return _basicBlocks; }

private:
    Module* _parent;
    std::string _name;
    std::vector<Value*> _params;
    std::vector<BasicBlock*> _basicBlocks;
};
}  // namespace AtomIR
}  // namespace ATC

#endif