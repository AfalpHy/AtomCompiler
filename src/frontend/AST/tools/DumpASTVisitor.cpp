
#include "AST/tools/DumpASTVisitor.h"

#include <iostream>
#include <unordered_map>

#include "AST/CompUnit.h"
#include "AST/Decl.h"
#include "AST/Expression.h"
#include "AST/FuncDef.h"
#include "AST/Statement.h"
#include "AST/Variable.h"

using namespace std;

namespace ATC {

unordered_map<int, string> ClassName = {
    // clang-format off
    {ID_COMP_UNIT, "CompUint"},
    {ID_DECL, "Decl"},
    {ID_FUNC_DEF, "FuncDef"},
    {ID_VARIABLE, "Variable"},
    {ID_EXPRESSION, "Expression"},
    {ID_CONST_VAL, "ConstVal"},
    {ID_VAR_REF, "VarRef"},
    {ID_UNARY_EXPRESSION, "UnaryExpression"},
    {ID_BINARY_EXPRESSION, "BinaryExpression"},
    {ID_FUNC_CALL, "FuncCall"},
    {ID_STATEMENT, "Statement"},
    {ID_BLOCK, "Block"},
    {ID_ASSIGN_STATEMENT, "AssignStatement"},
    {ID_BLANK_STATEMENT, "BlankStatement"},
    {ID_IF_STATEMENT, "IfStatement"},
    {ID_ELSE_STATEMENT, "ElseStatement"},
    {ID_WHILE_STATEMENT, "WhileStatement"},
    {ID_BREAK_STATEMENT, "BreakStatement"},
    {ID_CONTINUE_STATEMENT, "ContinueStatement"},
    {ID_RETURN_STATEMENT, "ReturnStatement"}
    // clang-format on
};

void DumpASTVisitor::printNode(TreeNode* node) {
    for (int i = 0; i < _indent; i++) {
        cout << "  ";
    }
    Position position = node->getPosition();
    printf("%s <%d:%d-%d:%d> %p", ClassName[node->getClassId()].c_str(), position._leftLine,
           position._leftColumn, position._rightLine, position._rightColumn, node);
}

void DumpASTVisitor::visit(TreeNode* node) {}

void DumpASTVisitor::visit(CompUnit* node) {
    printNode(node);
    cout << endl;
    _indent++;
    for (auto element : node->getElements()) {
        element->accept(this);
    }
    _indent--;
}

void DumpASTVisitor::visit(Decl* node) {
    printNode(node);
    cout << endl;
    _indent++;
    node->getVariable()->accept(this);
    _indent--;
}

void DumpASTVisitor::visit(FuncDef* node) {
    printNode(node);
    cout << endl;
    _indent++;
    node->getBlock()->accept(this);
    _indent--;
}

void DumpASTVisitor::visit(Variable* node) {
    printNode(node);
    cout << " " << node->getName() << " ";
    cout << " " << node->getDataType()->getBaseDataType() << " " << endl;
    _indent++;
    if (node->getInitValue()) {
        node->getInitValue()->accept(this);
    }
    _indent--;
}

// void DumpASTVisitor::visit(Expression* node) {}

void DumpASTVisitor::visit(ConstVal* node) {
    printNode(node);
    cout << " <" << node->getIntValue() << ">" << endl;
}

void DumpASTVisitor::visit(VarRef* node) {
    printNode(node);
    cout << " ref to " << node->getVariable() << endl;
    _indent++;
    for (auto dimension : node->getDimensions()) {
        dimension->accept(this);
    }
    _indent--;
}

void DumpASTVisitor::visit(UnaryExpression* node) {}

void DumpASTVisitor::visit(BinaryExpression* node) {}

// void DumpASTVisitor::visit(Statement* node) {}

void DumpASTVisitor::visit(Block* node) {
    printNode(node);
    cout << endl;
    _indent++;
    for (auto element : node->getElements()) {
        element->accept(this);
    }
    _indent--;
}

void DumpASTVisitor::visit(AssignStatement* node) {
    printNode(node);
    cout << endl;
    _indent++;
    node->getVar()->accept(this);
    node->getValue()->accept(this);
    _indent--;
}

void DumpASTVisitor::visit(IfStatement* node) {}

void DumpASTVisitor::visit(ReturnStatement* node) {
    printNode(node);
    cout << endl;
    _indent++;
    node->getExpr()->accept(this);
    _indent--;
}
}  // namespace ATC
