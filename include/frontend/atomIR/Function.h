#ifndef ATOM_FUNCTION_H
#define ATOM_FUNCTION_H

#include <set>

#include "BasicBlock.h"
#include "Value.h"

namespace ATC {
namespace AtomIR {

class Module;

struct FunctionType {
    std::vector<Type*> _params;
    Type* _ret;
};

class Function {
public:
    Function(Module* parent, const FunctionType& functionType, const std::string& name)
        : _parent(parent), _functionType(functionType), _name(name) {}

    void addParam(Value* param) { _params.push_back(param); }
    void insertBB(BasicBlock* bb) { _basicBlocks.push_back(bb); }
    void setNeedUpdateName(bool b) { _needUpdateName = b; }

    Module* getParent() { return _parent; }

    const std::string& getName() { return _name; }

    const FunctionType& getFunctionType() { return _functionType; }

    BasicBlock* getEnteryBB() {
        if (_basicBlocks.size() > 0) {
            return _basicBlocks[0];
        }
        return nullptr;
    }

    const std::vector<Value*>& getParams() { return _params; }
    const std::vector<BasicBlock*>& getBasicBlocks() { return _basicBlocks; }

    std::string getUniqueNameInFunction(const std::string& name);

    void updateNameIfNeed();

    void dump();

private:
    Module* _parent;
    std::string _name;
    FunctionType _functionType;
    std::vector<Value*> _params;
    std::vector<BasicBlock*> _basicBlocks;
    std::set<std::string> _nameSet;  // value and block name set
    int _valueIndex = 0;
    bool _needUpdateName = true;  // Ture if the name set need update
};
}  // namespace AtomIR
}  // namespace ATC

#endif