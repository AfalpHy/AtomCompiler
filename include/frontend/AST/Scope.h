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

    Variable* getVariable(const std::string& name) {
        if (varMap.find(name) != varMap.end()) {
            return varMap[name];
        }
        if (_parent) {
            return _parent->getVariable(name);
        }
        return nullptr;
    }
    Scope* getParent() { return _parent; }

    void setParent(Scope* parent) { _parent = parent; }

    void insertVariable(const std::string& name, Variable* var) { varMap.insert({name, var}); }

private:
    Scope* _parent;
    std::unordered_map<std::string, Variable*> varMap;
};

}  // namespace ATC

#endif