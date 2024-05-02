#pragma once

#include <set>
#include <unordered_map>

#include "BasicBlock.h"

namespace ATC {
namespace AtomIR {

class Module;

struct FuncTyHash {
    int operator()(const std::pair<Type*, std::vector<Type*>>& funcTy) const {
        uint64_t ret = (uint64_t)funcTy.first;
        for (auto ty : funcTy.second) {
            ret += (uint64_t)ty;
        }
        std::hash<uint64_t> helper;
        return helper(ret);
    }
};

struct FunctionType {
    std::vector<Type*> _params;
    Type* _ret;

    static FunctionType* get(Type* ret, std::vector<Type*> params, bool IsVarArgs) {
        static std::unordered_map<std::pair<Type*, std::vector<Type*>>, FunctionType*, FuncTyHash> funcTy2ptr;
        if (funcTy2ptr.find({ret, params}) != funcTy2ptr.end()) {
            return funcTy2ptr[{ret, params}];
        }
        FunctionType* funcTyPtr = new FunctionType();
        funcTyPtr->_ret = ret;
        funcTyPtr->_params = params;
        funcTy2ptr.insert({{ret, params}, funcTyPtr});
        return funcTyPtr;
    }

private:
    FunctionType() {}
};

class Function {
public:
    Function(Module* parent, const FunctionType& functionType, const std::string& name);

    void addParam(Value* param) { _params.push_back(param); }

    void insertBB(BasicBlock* bb) { _basicBlocks.push_back(bb); }

    void setHasFunctionCall(bool b) { _hasFunctionCall = b; }

    Module* getParent() { return _parent; }

    const std::string& getName() { return _name; }

    const FunctionType& getFunctionType() { return _functionType; }

    const std::vector<Value*>& getParams() { return _params; }

    const std::vector<BasicBlock*>& getBasicBlocks() { return _basicBlocks; }

    bool hasFunctionCall() { return _hasFunctionCall; }

    std::string getUniqueNameInFunction(void* ptr);

    void insertName(Value* value);

    void insertName(BasicBlock* bb);

    void insertName(void* ptr, const std::string& name);

    std::list<Instruction*>::iterator& getCurAllocIter() { return _currentAllocIter; }

    std::string toString();

    void dump();

private:
    Module* _parent;
    std::string _name;
    FunctionType _functionType;
    std::vector<Value*> _params;
    std::vector<BasicBlock*> _basicBlocks;
    std::set<std::string> _nameSet;                   // value and block name set
    std::unordered_map<void*, std::string> _nameMap;  // value and block pointer to unique name
    bool _hasFunctionCall = false;                    // Ture if there are some functionCall in this function
    int _valueIndex = 0;
    std::list<Instruction*>::iterator _currentAllocIter;
};
}  // namespace AtomIR
}  // namespace ATC
