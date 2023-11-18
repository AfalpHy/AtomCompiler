#include "AST/SemanticChecker.h"

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

// void SemanticChecker::visit(CompUnit* node) { AtomASTVisitor::visit(node); }

// void SemanticChecker::visit(Decl* node) { AtomASTVisitor::visit(node); }

// void SemanticChecker::visit(FunctionDef* node) { AtomASTVisitor::visit(node); }

// void SemanticChecker::visit(DataType* node) {}

// void SemanticChecker::visit(Variable* node) { AtomASTVisitor::visit(node); }

// void SemanticChecker::visit(ConstVal* node) {}

void SemanticChecker::visit(VarRef* node) {
    if (node->getVariable() == nullptr) {
        Position position = node->getPosition();
        printf("variable %s %p <%d:%d-%d:%d> not defined\n", node->getName().c_str(), node, position._leftLine,
               position._leftColumn, position._rightLine, position._rightColumn);
        exit(0);
    }
    AtomASTVisitor::visit(node);
}

void SemanticChecker::visit(IndexedRef* node) {
    if (node->getVariable() == nullptr) {
        Position position = node->getPosition();
        printf("variable %s %p <%d:%d-%d:%d> not defined\n", node->getName().c_str(), node, position._leftLine,
               position._leftColumn, position._rightLine, position._rightColumn);
        exit(0);
    }
    AtomASTVisitor::visit(node);
}

// void SemanticChecker::visit(NestedExpression* node) { AtomASTVisitor::visit(node); }

// void SemanticChecker::visit(UnaryExpression* node) { AtomASTVisitor::visit(node); }

// void SemanticChecker::visit(BinaryExpression* node) { AtomASTVisitor::visit(node); }

void SemanticChecker::visit(FunctionCall* node) {
    if (node->getFunctionDef() == nullptr) {
        Position position = node->getPosition();
        printf("function %s %p <%d:%d-%d:%d> not defined\n", node->getName().c_str(), node, position._leftLine,
               position._leftColumn, position._rightLine, position._rightColumn);
        exit(0);
    }
    AtomASTVisitor::visit(node);
}

// void SemanticChecker::visit(Block* node) { AtomASTVisitor::visit(node); }

// void SemanticChecker::visit(AssignStatement* node) { AtomASTVisitor::visit(node); }

// void SemanticChecker::visit(IfStatement* node) { AtomASTVisitor::visit(node); }

// void SemanticChecker::visit(ElseStatement* node) { AtomASTVisitor::visit(node); }

// void SemanticChecker::visit(WhileStatement* node) { AtomASTVisitor::visit(node); }

// void SemanticChecker::visit(ReturnStatement* node) { AtomASTVisitor::visit(node); }

// void SemanticChecker::visit(OtherStatement* node) { AtomASTVisitor::visit(node); }
}  // namespace ATC
