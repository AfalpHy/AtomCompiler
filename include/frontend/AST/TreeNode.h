#pragma once

#include <string>

#include "ASTVisitor.h"
#include "antlr4-runtime.h"

#define ACCEPT \
    virtual void accept(ASTVisitor* visitor) { visitor->visit(this); }
namespace ATC {

class ASTVisitor;
class Scope;

enum NodeType {
    ID_COMP_UNIT,

    ID_BASIC_TYPE,
    ID_ARRAY_TYPE,
    ID_POINTER_TYPE,

    ID_VAR_DECL,
    ID_FUNCTION_DECL,
    ID_FUNCTION_DEF,
    ID_VARIABLE,

    // expression
    ID_CONST_VAL,
    ID_VAR_REF,
    ID_INDEXED_REF,
    ID_NESTED_EXPRESSION,
    ID_UNARY_EXPRESSION,
    ID_BINARY_EXPRESSION,
    ID_FUNCTION_CALL,

    // statement
    ID_BLOCK,
    ID_ASSIGN_STATEMENT,
    ID_BLANK_STATEMENT,
    ID_IF_STATEMENT,
    ID_ELSE_STATEMENT,
    ID_WHILE_STATEMENT,
    ID_BREAK_STATEMENT,
    ID_CONTINUE_STATEMENT,
    ID_RETURN_STATEMENT,
    ID_OTHER_STATEMENT
};

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

    virtual int getClassId() = 0;

    std::string getName() { return _name; }
    Position getPosition() { return _position; }
    Scope* getScope() { return _scope; }

    void setName(std::string name) { _name = name; }
    void setPosition(antlr4::Token* start, antlr4::Token* stop);
    void setScope(Scope* scope) { _scope = scope; }

    ACCEPT

protected:
    std::string _name;
    Position _position;
    Scope* _scope = nullptr;
};

}  // namespace ATC
