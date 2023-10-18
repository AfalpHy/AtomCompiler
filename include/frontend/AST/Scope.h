#ifndef SCOPE_H
#define SCOPE_H

#include <string>
#include <unordered_map>
#include <vector>

#include "Variable.h"

namespace ATC {

class Scope {
public:
    Scope() = default;

    Scope* getParent() { return _parent; }
    const std::vector<Scope*>& getChildren() { return _children; }

    Variable* getVariable(const std::string& name);
    FunctionDef* getFunctionDef(const std::string& name);
    const std::unordered_map<std::string, Variable*>& getVarMap() { return _varMap; }
    void setParent(Scope* parent) { _parent = parent; }
    void addChild(Scope* child) { _children.push_back(child); }

    void insertVariable(const std::string& name, Variable* var);
    void insertFunctionDef(const std::string& name, FunctionDef* functionDef);

private:
    Scope* _parent;
    std::vector<Scope*> _children;
    std::unordered_map<std::string, Variable*> _varMap;
    std::unordered_map<std::string, FunctionDef*> _functionMap;
};

extern Scope* CurrentScope;
}  // namespace ATC

#endif