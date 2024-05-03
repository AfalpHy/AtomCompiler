
#include "AST/ASTVisitor.h"

#include <filesystem>
#include <iostream>
#include <unordered_map>

#include "AST/CompUnit.h"
#include "AST/Expression.h"
#include "AST/Function.h"
#include "AST/Statement.h"
#include "AST/Variable.h"

using namespace std;

namespace ATC {

void ASTVisitor::visit(TreeNode* node) {}

void ASTVisitor::visit(CompUnit* node) {
    for (auto element : node->getElements()) {
        element->accept(this);
    }
}

void ASTVisitor::visit(VarDecl* node) {
    for (auto var : node->getVariables()) {
        var->accept(this);
    }
}

void ASTVisitor::visit(FunctionDecl* node) {
    for (auto fParam : node->getParams()) {
        fParam->accept(this);
    }
}

void ASTVisitor::visit(FunctionDef* node) {
    node->getFunctionDecl()->accept(this);
    node->getBlock()->accept(this);
}

void ASTVisitor::visit(BasicType* node) {}

void ASTVisitor::visit(ArrayType* node) { node->getBaseDataType()->accept(this); }

void ASTVisitor::visit(PointerType* node) { node->getBaseDataType()->accept(this); }

void ASTVisitor::visit(Variable* node) {
    node->getDataType()->accept(this);
    if (node->getInitValue()) {
        node->getInitValue()->accept(this);
    }
}

void ASTVisitor::visit(ConstVal* node) {}

void ASTVisitor::visit(VarRef* node) {}

void ASTVisitor::visit(IndexedRef* node) {
    for (auto dimension : node->getDimensions()) {
        dimension->accept(this);
    }
}

void ASTVisitor::visit(NestedExpression* node) {
    for (auto expr : node->getElements()) {
        expr->accept(this);
    }
}

void ASTVisitor::visit(UnaryExpression* node) { node->getOperand()->accept(this); }

void ASTVisitor::visit(BinaryExpression* node) {
    node->getLeft()->accept(this);
    node->getRight()->accept(this);
}

void ASTVisitor::visit(FunctionCall* node) {
    for (auto rParam : node->getParams()) {
        rParam->accept(this);
    }
}

void ASTVisitor::visit(Block* node) {
    for (auto element : node->getElements()) {
        element->accept(this);
    }
}

void ASTVisitor::visit(AssignStatement* node) {
    node->getLval()->accept(this);
    node->getRval()->accept(this);
}

void ASTVisitor::visit(IfStatement* node) {
    node->getCond()->accept(this);
    node->getStmt()->accept(this);
    if (node->getElseStmt()) {
        node->getElseStmt()->accept(this);
    }
}

void ASTVisitor::visit(ElseStatement* node) { node->getStmt()->accept(this); }

void ASTVisitor::visit(WhileStatement* node) {
    node->getCond()->accept(this);
    node->getStmt()->accept(this);
}

void ASTVisitor::visit(BreakStatement*) {}

void ASTVisitor::visit(ContinueStatement*) {}

void ASTVisitor::visit(ReturnStatement* node) {
    if (node->getExpr()) {
        node->getExpr()->accept(this);
    }
}

void ASTVisitor::visit(OtherStatement* node) { node->getExpr()->accept(this); }
}  // namespace ATC
