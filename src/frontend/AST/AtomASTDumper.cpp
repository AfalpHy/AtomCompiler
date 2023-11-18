
#include "AST/AtomASTDumper.h"

#include <filesystem>
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
    {ID_BASIC_TYPE,             "BasicType"},
    {ID_ARRAY_TYPE,             "ArrayType"},
    {ID_POINTER_TYPE,           "PointerType"},
    {ID_VAR_DECL,               "VarDecl"},
    {ID_FUNCTION_DEF,           "FunctionDef"},
    {ID_VARIABLE,               "Variable"},
    {ID_CONST_VAL,              "ConstVal"},
    {ID_VAR_REF,                "VarRef"},
    {ID_INDEXED_REF,            "IndexedRef"},
    {ID_NESTED_EXPRESSION,      "NestedExpression"},
    {ID_UNARY_EXPRESSION,       "UnaryExpression"},
    {ID_BINARY_EXPRESSION,      "BinaryExpression"},
    {ID_FUNCTION_CALL,          "FunctionCall"},
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

unordered_map<int, string> TypeName = {
    // clang-format off
    {BasicType::Type::UNKOWN,    "unkown"},
    {BasicType::Type::BOOL,      "bool"},
    {BasicType::Type::INT,       "int"},
    {BasicType::Type::FLOAT,     "float"},
    {BasicType::Type::VOID,      "void"}
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
    {LT,    "LT"},
    {GT,    "GT"},
    {LE,    "LE"},
    {GE,    "GE"},
    {EQ,    "EQ"},
    {NE,    "NE"},
    {AND,   "And"},
    {OR,    "Or"}
    // clang-format on
};

void AtomASTDumper::printNode(TreeNode* node, bool newLine) {
    for (int i = 0; i < _indent; i++) {
        cout << "  ";
    }
    Position position = node->getPosition();
    printf("%s %p <%d:%d-%d:%d> %s", ClassName[node->getClassId()].c_str(), node, position._leftLine,
           position._leftColumn, position._rightLine, position._rightColumn, node->getName().c_str());
    if (newLine) {
        cout << endl;
    }
}

void AtomASTDumper::visit(TreeNode* node) { printNode(node); }

void AtomASTDumper::visit(CompUnit* node) {
    cout << filesystem::absolute(node->getPosition()._fileName) << endl;
    printNode(node);
    _indent++;
    AtomASTVisitor::visit(node);
    _indent--;
}

void AtomASTDumper::visit(VarDecl* node) {
    printNode(node);
    _indent++;
    AtomASTVisitor::visit(node);
    _indent--;
}

void AtomASTDumper::visit(FunctionDef* node) {
    printNode(node);
    _indent++;
    AtomASTVisitor::visit(node);
    _indent--;
}

void AtomASTDumper::visit(BasicType* node) {
    for (int i = 0; i < _indent; i++) {
        cout << "  ";
    }
    Position position = node->getPosition();
    printf("%s %p ", ClassName[node->getClassId()].c_str(), node);
    cout << "<" << TypeName[node->getBasicType()] << ">" << endl;
}

void AtomASTDumper::visit(ArrayType* node) {
    for (int i = 0; i < _indent; i++) {
        cout << "  ";
    }
    Position position = node->getPosition();
    printf("%s %p ", ClassName[node->getClassId()].c_str(), node);
    cout << "<";
    for (auto dimension : node->getDimensions()) {
        cout << "[" << dimension << "]";
    }
    cout << ">" << endl;
    _indent++;
    AtomASTVisitor::visit(node);
    _indent--;
}

void AtomASTDumper::visit(PointerType* node) {
    for (int i = 0; i < _indent; i++) {
        cout << "  ";
    }
    Position position = node->getPosition();
    printf("%s %p ", ClassName[node->getClassId()].c_str(), node);
    cout << "<Pointer>" << endl;
    _indent++;
    AtomASTVisitor::visit(node);
    _indent--;
}

void AtomASTDumper::visit(Variable* node) {
    printNode(node, false);
    cout << " ";
    cout << (node->isConst() ? " const" : "") << endl;
    _indent++;
    AtomASTVisitor::visit(node);
    _indent--;
}

void AtomASTDumper::visit(ConstVal* node) {
    printNode(node, false);
    if (node->getBasicType() == BasicType::INT) {
        cout << "<" << node->getIntValue() << ">" << endl;
    } else {
        cout << "<" << node->getFloatValue() << ">" << endl;
    }
}

void AtomASTDumper::visit(VarRef* node) {
    printNode(node, false);
    cout << " ref to " << node->getVariable() << endl;
}

void AtomASTDumper::visit(IndexedRef* node) {
    printNode(node, false);
    cout << " ref to " << node->getVariable() << endl;
    _indent++;
    AtomASTVisitor::visit(node);
    _indent--;
}

void AtomASTDumper::visit(NestedExpression* node) {
    printNode(node);
    _indent++;
    AtomASTVisitor::visit(node);
    _indent--;
}

void AtomASTDumper::visit(UnaryExpression* node) {
    printNode(node, false);
    cout << "<" << OperatorName[node->getOperator()] << ">" << endl;
    _indent++;
    AtomASTVisitor::visit(node);
    _indent--;
}

void AtomASTDumper::visit(BinaryExpression* node) {
    printNode(node, false);
    cout << "<" << OperatorName[node->getOperator()] << ">" << endl;
    _indent++;
    AtomASTVisitor::visit(node);
    _indent--;
}

void AtomASTDumper::visit(FunctionCall* node) {
    printNode(node);
    _indent++;
    AtomASTVisitor::visit(node);
    _indent--;
}

void AtomASTDumper::visit(Block* node) {
    printNode(node);
    _indent++;
    AtomASTVisitor::visit(node);
    _indent--;
}

void AtomASTDumper::visit(AssignStatement* node) {
    printNode(node);
    _indent++;
    AtomASTVisitor::visit(node);
    _indent--;
}

void AtomASTDumper::visit(IfStatement* node) {
    printNode(node);
    _indent++;
    AtomASTVisitor::visit(node);
    _indent--;
}

void AtomASTDumper::visit(ElseStatement* node) {
    printNode(node);
    _indent++;
    AtomASTVisitor::visit(node);
    _indent--;
}

void AtomASTDumper::visit(WhileStatement* node) {
    printNode(node);
    _indent++;
    AtomASTVisitor::visit(node);
    _indent--;
}

void AtomASTDumper::visit(ReturnStatement* node) {
    printNode(node);
    _indent++;
    AtomASTVisitor::visit(node);
    _indent--;
}

void AtomASTDumper::visit(OtherStatement* node) {
    printNode(node);
    _indent++;
    AtomASTVisitor::visit(node);
    _indent--;
}
}  // namespace ATC
