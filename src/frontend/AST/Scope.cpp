#include "AST/Scope.h"

#include "AST/Function.h"

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

FunctionDecl* Scope::getFunction(const std::string& name) {
    if (_functionMap.find(name) != _functionMap.end()) {
        return _functionMap[name];
    }
    if (_parent) {
        return _parent->getFunction(name);
    }
    return nullptr;
}

void Scope::insertVariable(const std::string& name, Variable* var) { _varMap.insert({name, var}); }

void Scope::insertFunction(const std::string& name, FunctionDecl* functionDecl) {
    _functionMap.insert({name, functionDecl});
}

}  // namespace ATC