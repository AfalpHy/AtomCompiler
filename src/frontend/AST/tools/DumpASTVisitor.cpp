
#include "AST/tools/DumpASTVisitor.h"

#include <iostream>
#include <unordered_map>

#include "AST/CompUnit.h"
#include "AST/Decl.h"
#include "AST/Expression.h"
#include "AST/FunctionDef.h"
#include "AST/Statement.h"
#include "AST/Variable.h"

using namespace std;

namespace ATC {

unordered_map<int, string> ClassName = {
    // clang-format off
    {ID_COMP_UNIT,              "CompUint"},
    {ID_DECL,                   "Decl"},
    {ID_FUNCTION_DEF,           "FunctionDef"},
    {ID_VARIABLE,               "Variable"},
    {ID_EXPRESSION,             "Expression"},
    {ID_CONST_VAL,              "ConstVal"},
    {ID_VAR_REF,                "VarRef"},
    {ID_ARRAY_EXPRESSION,       "ArrayExpression"},
    {ID_UNARY_EXPRESSION,       "UnaryExpression"},
    {ID_BINARY_EXPRESSION,      "BinaryExpression"},
    {ID_FUNCTION_CALL,          "FunctionCall"},
    {ID_STATEMENT,              "Statement"},
    {ID_BLOCK,                  "Block"},
    {ID_ASSIGN_STATEMENT,       "AssignStatement"},
    {ID_BLANK_STATEMENT,        "BlankStatement"},
    {ID_IF_STATEMENT,           "IfStatement"},
    {ID_ELSE_STATEMENT,         "ElseStatement"},
    {ID_WHILE_STATEMENT,        "WhileStatement"},
    {ID_BREAK_STATEMENT,        "BreakStatement"},
    {ID_CONTINUE_STATEMENT,     "ContinueStatement"},
    {ID_RETURN_STATEMENT,       "ReturnStatement"},
    {ID_OTHER_STATEMENT,        "OtherStatement"}
    // clang-format on
};

unordered_map<int, string> BaseTypeName = {
    // clang-format off
    {UNKOWN,    "unkown"},
    {INT,       "int"},
    {FLOAT,     "float"},
    {VOID,      "void"}
    // clang-format on
};

unordered_map<int, string> OperatorName = {
    // clang-format off
    {PLUS,  "Plus"},
    {MINUS, "Minus"},
    {NOT,   "Not"},
    {MUL,   "Mul"},
    {DIV,   "Div"},
    {MOD,   "Mod"},
    {LT,    "Lt"},
    {GT,    "Gt"},
    {LE,    "Le"},
    {GE,    "Ge"},
    {EQ,    "Eq"},
    {NE,    "Ne"},
    {AND,   "And"},
    {OR,    "Or"}
    // clang-format on
};

void DumpASTVisitor::printNode(TreeNode* node, bool newLine) {
    for (int i = 0; i < _indent; i++) {
        cout << "  ";
    }
    Position position = node->getPosition();
    printf("%s %p <%d:%d-%d:%d> %s", ClassName[node->getClassId()].c_str(), node,
           position._leftLine, position._leftColumn, position._rightLine, position._rightColumn,
           node->getName().c_str());
    if (newLine) {
        cout << endl;
    }
}

void DumpASTVisitor::visit(TreeNode* node) { printNode(node); }

void DumpASTVisitor::visit(CompUnit* node) {
    printNode(node);
    _indent++;
    for (auto element : node->getElements()) {
        element->accept(this);
    }
    _indent--;
}

void DumpASTVisitor::visit(Decl* node) {
    printNode(node);
    _indent++;
    for (auto var : node->getVariables()) {
        var->accept(this);
    }
    _indent--;
}

void DumpASTVisitor::visit(FunctionDef* node) {
    printNode(node);
    _indent++;
    node->getBlock()->accept(this);
    _indent--;
}

void DumpASTVisitor::visit(DataType* node) {
    cout << "<" << BaseTypeName[node->getBaseType()];
    for (auto dimension : node->getDimensions()) {
        cout << "[" << Expression::evaluateConstExpr(dimension) << "]";
    }
    cout << ">";
}

void DumpASTVisitor::visit(Variable* node) {
    printNode(node, false);
    cout << " ";
    node->getDataType()->accept(this);
    cout << (node->isConst() ? " const" : "") << endl;
    _indent++;
    if (node->getInitValue()) {
        node->getInitValue()->accept(this);
    }
    _indent--;
}

void DumpASTVisitor::visit(ConstVal* node) {
    printNode(node, false);
    if (node->getBaseType() == INT) {
        cout << "<" << node->getIntValue() << ">" << endl;
    } else {
        cout << "<" << node->getFloatValue() << ">" << endl;
    }
}

void DumpASTVisitor::visit(VarRef* node) {
    printNode(node, false);
    cout << " ref to " << node->getVariable() << endl;
    _indent++;
    for (auto dimension : node->getDimensions()) {
        dimension->accept(this);
    }
    _indent--;
}

void DumpASTVisitor::visit(ArrayExpression* node) {
    printNode(node);
    _indent++;
    for (auto expr : node->getElements()) {
        expr->accept(this);
    }
    _indent--;
}

void DumpASTVisitor::visit(UnaryExpression* node) {
    printNode(node, false);
    cout << "<" << OperatorName[node->getOperator()] << ">" << endl;
    _indent++;
    node->getOperand()->accept(this);
    _indent--;
}

void DumpASTVisitor::visit(BinaryExpression* node) {
    printNode(node, false);
    cout << "<" << OperatorName[node->getOperator()] << ">" << endl;
    _indent++;
    node->getLeft()->accept(this);
    node->getRight()->accept(this);
    _indent--;
}

void DumpASTVisitor::visit(FunctionCall* node) {
    printNode(node, false);
    cout << node->getName() << endl;
    _indent++;
    for (auto rParam : node->getParams()) {
        rParam->accept(this);
    }
    _indent--;
}

void DumpASTVisitor::visit(Block* node) {
    printNode(node);
    _indent++;
    for (auto element : node->getElements()) {
        element->accept(this);
    }
    _indent--;
}

void DumpASTVisitor::visit(AssignStatement* node) {
    printNode(node);
    _indent++;
    node->getVar()->accept(this);
    node->getValue()->accept(this);
    _indent--;
}

void DumpASTVisitor::visit(IfStatement* node) {
    printNode(node);
    _indent++;
    node->getCond()->accept(this);
    node->getStmt()->accept(this);
    if (node->getElseStmt()) {
        node->getElseStmt()->accept(this);
    }
    _indent--;
}

void DumpASTVisitor::visit(ElseStatement* node) {
    printNode(node);
    _indent++;
    node->getStmt()->accept(this);
    _indent--;
}

void DumpASTVisitor::visit(WhileStatement* node) {
    printNode(node);
    _indent++;
    node->getCond()->accept(this);
    node->getStmt()->accept(this);
    _indent--;
}

void DumpASTVisitor::visit(ReturnStatement* node) {
    printNode(node);
    if (node->getExpr()) {
        _indent++;
        node->getExpr()->accept(this);
        _indent--;
    }
}

void DumpASTVisitor::visit(OtherStatement* node) {
    printNode(node);
    _indent++;
    node->getExpr()->accept(this);
    _indent--;
}
}  // namespace ATC
