#include "AST/tools/SemanticChecker.h"

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

// void SemanticChecker::visit(TreeNode* node) {}

// void SemanticChecker::visit(CompUnit* node) { ASTVisitor::visit(node); }

// void SemanticChecker::visit(Decl* node) { ASTVisitor::visit(node); }

// void SemanticChecker::visit(FunctionDef* node) { ASTVisitor::visit(node); }

// void SemanticChecker::visit(DataType* node) {}

// void SemanticChecker::visit(Variable* node) { ASTVisitor::visit(node); }

// void SemanticChecker::visit(ConstVal* node) {}

void SemanticChecker::visit(VarRef* node) {
    if (node->getVariable() == nullptr) {
        Position position = node->getPosition();
        printf("variable %s %p <%d:%d-%d:%d> not defined\n", node->getName().c_str(), node, position._leftLine,
               position._leftColumn, position._rightLine, position._rightColumn);
        exit(0);
    }
    ASTVisitor::visit(node);
}

void SemanticChecker::visit(IndexedRef* node) {
    if (node->getVariable() == nullptr) {
        Position position = node->getPosition();
        printf("variable %s %p <%d:%d-%d:%d> not defined\n", node->getName().c_str(), node, position._leftLine,
               position._leftColumn, position._rightLine, position._rightColumn);
        exit(0);
    }
    ASTVisitor::visit(node);
}

// void SemanticChecker::visit(NestedExpression* node) { ASTVisitor::visit(node); }

// void SemanticChecker::visit(UnaryExpression* node) { ASTVisitor::visit(node); }

// void SemanticChecker::visit(BinaryExpression* node) { ASTVisitor::visit(node); }

void SemanticChecker::visit(FunctionCall* node) {
    if (node->getFunctionDef() == nullptr) {
        Position position = node->getPosition();
        printf("function %s %p <%d:%d-%d:%d> not defined\n", node->getName().c_str(), node, position._leftLine,
               position._leftColumn, position._rightLine, position._rightColumn);
        exit(0);
    }
    ASTVisitor::visit(node);
}

// void SemanticChecker::visit(Block* node) { ASTVisitor::visit(node); }

// void SemanticChecker::visit(AssignStatement* node) { ASTVisitor::visit(node); }

// void SemanticChecker::visit(IfStatement* node) { ASTVisitor::visit(node); }

// void SemanticChecker::visit(ElseStatement* node) { ASTVisitor::visit(node); }

// void SemanticChecker::visit(WhileStatement* node) { ASTVisitor::visit(node); }

// void SemanticChecker::visit(ReturnStatement* node) { ASTVisitor::visit(node); }

// void SemanticChecker::visit(OtherStatement* node) { ASTVisitor::visit(node); }
}  // namespace ATC
