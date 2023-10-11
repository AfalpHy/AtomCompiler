#ifndef TREE_NODE_H
#define TREE_NODE_H

#include <string>

#include "../../Common.h"
#include "antlr4-runtime.h"
#include "tools/ASTVisitor.h"

#define ACCEPT virtual void accept(ASTVisitor* visitor) { visitor->visit(this); }
namespace ATC {

class ASTVisitor;
typedef struct {
    std::string _fileName;
    int _leftLine;
    int _leftColumn;
    int _rightLine;
    int _rightColumn;
} Position;

class TreeNode {
public:
    TreeNode() = default;

    virtual int getClassId() = 0;

    TreeNode* getParent() { return _parent; }
    std::string getName() { return _name; }
    Position getPosition() { return _position; }

    void setName(std::string name) { _name = name; }

    void setPosition(antlr4::Token* start, antlr4::Token* stop);

    ACCEPT

private:
    TreeNode* _parent = nullptr;
    std::string _name;
    Position _position;
};

}  // namespace ATC

#endif