#include "AST/Scope.h"

#include "AST/FunctionDef.h"

namespace ATC {

Variable* Scope::getVariable(const std::string& name) {
    if (_varMap.find(name) != _varMap.end()) {
        return _varMap[name];
    }
    if (_parent) {
        return _parent->getVariable(name);
    }
    return nullptr;
}

FunctionDef* Scope::getFunctionDef(const std::string& name) {
    if (_functionMap.find(name) != _functionMap.end()) {
        return _functionMap[name];
    }
    if (_parent) {
        return _parent->getFunctionDef(name);
    }
    return nullptr;
}

void Scope::insertVariable(const std::string& name, Variable* var) { _varMap.insert({name, var}); }

void Scope::insertFunctionDef(const std::string& name, FunctionDef* functionDef) {
    _functionMap.insert({name, functionDef});
}


}  // namespace ATC