#ifndef TREE_NODE_H
#define TREE_NODE_H

#include <string>

#include "../../Common.h"
#include "antlr4-runtime.h"
#include "tools/ASTVisitor.h"

#define ACCEPT \
    virtual void accept(ASTVisitor* visitor) { visitor->visit(this); }
namespace ATC {

class ASTVisitor;
class Scope;

typedef struct {
    std::string _fileName = "unknow";
    int _leftLine = 0;
    int _leftColumn = 0;
    int _rightLine = 0;
    int _rightColumn = 0;
} Position;

class TreeNode {
public:
    TreeNode() = default;
    TreeNode(TreeNode* parent) : _parent(parent) {}
    
    virtual int getClassId() = 0;

    TreeNode* getParent() { return _parent; }
    std::string getName() { return _name; }
    Position getPosition() { return _position; }
    Scope* getScope() { return _scope; }

    void setParent(TreeNode* parent) { _parent = parent; }
    void setName(std::string name) { _name = name; }
    void setPosition(antlr4::Token* start, antlr4::Token* stop);
    void setScope(Scope* scope) { _scope = scope; }

    ACCEPT

protected:
    TreeNode* _parent = nullptr;
    std::string _name;
    Position _position;
    Scope* _scope = nullptr;
};

}  // namespace ATC

#endif