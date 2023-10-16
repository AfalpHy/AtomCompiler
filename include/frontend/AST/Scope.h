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

    void setParent(Scope* parent) { _parent = parent; }
    void addChild(Scope* child) { _children.push_back(child); }
    void addNeedFixupNode(TreeNode* node) { _needFixupNode.push_back(node); }

    void insertVariable(const std::string& name, Variable* var);
    void insertFunctionDef(const std::string& name, FunctionDef* functionDef);

    void fixupNode();

private:
    Scope* _parent;
    std::vector<Scope*> _children;
    std::unordered_map<std::string, Variable*> varMap;
    std::unordered_map<std::string, FunctionDef*> functionMap;
    std::vector<TreeNode*> _needFixupNode;
};

extern Scope* CurrentScope;
}  // namespace ATC

#endif