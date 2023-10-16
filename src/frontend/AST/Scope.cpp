#include "AST/Scope.h"

#include "AST/FunctionDef.h"

namespace ATC {

Variable* Scope::getVariable(const std::string& name) {
    if (varMap.find(name) != varMap.end()) {
        return varMap[name];
    }
    if (_parent) {
        return _parent->getVariable(name);
    }
    return nullptr;
}

FunctionDef* Scope::getFunctionDef(const std::string& name) {
    if (functionMap.find(name) != functionMap.end()) {
        return functionMap[name];
    }
    if (_parent) {
        return _parent->getFunctionDef(name);
    }
    return nullptr;
}

void Scope::insertVariable(const std::string& name, Variable* var) { varMap.insert({name, var}); }

void Scope::insertFunctionDef(const std::string& name, FunctionDef* functionDef) {
    functionMap.insert({name, functionDef});
}

void Scope::fixupNode() {
    for (auto node : _needFixupNode) {
        if (node->getClassId() == ID_VAR_REF) {
            auto varRef = (VarRef*)node;
            varRef->setVariable(getVariable(varRef->getName()));
        } else if (node->getClassId() == ID_FUNCTION_CALL) {
            auto functionCall = (FunctionCall*)node;
            functionCall->setFunctionDef(getFunctionDef(functionCall->getName()));
        }
    }
    for (auto child : _children) {
        child->fixupNode();
    }
}

}  // namespace ATC