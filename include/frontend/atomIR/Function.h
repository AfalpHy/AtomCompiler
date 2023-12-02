#ifndef ATC_FUNCTION_H
#define ATC_FUNCTION_H

#include <set>
#include <unordered_map>

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
    Function(Module* parent, const FunctionType& functionType, const std::string& name);

    void addParam(Value* param) { _params.push_back(param); }

    void insertBB(BasicBlock* bb) {
        insertName(bb);
        _basicBlocks.push_back(bb);
    }

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

    std::string getUniqueNameInFunction(void* ptr);

    void updateNameIfNeed();

    void dump();

    void insertName(Value* value);

    void insertName(BasicBlock* bb);

    void insertName(void* ptr, const std::string& name);

private:
    Module* _parent;
    std::string _name;
    FunctionType _functionType;
    std::vector<Value*> _params;
    std::vector<BasicBlock*> _basicBlocks;
    std::set<std::string> _nameSet;                   // value and block name set
    std::unordered_map<void*, std::string> _nameMap;  // value and block pointer to unique name
    int _valueIndex = 0;
    bool _needUpdateName = false;  // Ture if the name set need update
};
}  // namespace AtomIR
}  // namespace ATC

#endif