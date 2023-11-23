#ifndef MODULE_H
#define MODULE_H

#include <unordered_map>
#include <vector>

#include "Function.h"
#include "Value.h"

namespace ATC {
namespace AtomIR {

class Module {
public:
    Module(const std::string& name) { _name = name; }

    void addFunction(Function* function) { _functions.insert({function->getName(), function}); }
    void addGlobalVariable(Value* var) { _globalVariables.insert({var->getName(), var}); }

    const std::string& getName() { return _name; }
    const std::unordered_map<std::string, Function*>& getFunctions() { return _functions; }
    const std::unordered_map<std::string, Value*>& getGlobalVariables() { return _globalVariables; }

    Function* getFunction(const std::string& name) {
        if (_functions.find(name) != _functions.end()) {
            return _functions[name];
        }
        return nullptr;
    }

    Value* getGlobalVariable(const std::string& name) {
        if (_globalVariables.find(name) != _globalVariables.end()) {
            return _globalVariables[name];
        }
        return nullptr;
    }

private:
    std::string _name;
    std::unordered_map<std::string, Function*> _functions;
    std::unordered_map<std::string, Value*> _globalVariables;
};
}  // namespace AtomIR
}  // namespace ATC

#endif