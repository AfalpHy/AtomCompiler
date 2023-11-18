
#include "AST/AtomASTVisitor.h"

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

void AtomASTVisitor::visit(TreeNode* node) {}

void AtomASTVisitor::visit(CompUnit* node) {
    for (auto element : node->getElements()) {
        element->accept(this);
    }
}

void AtomASTVisitor::visit(VarDecl* node) {
    for (auto var : node->getVariables()) {
        var->accept(this);
    }
}

void AtomASTVisitor::visit(FunctionDef* node) {
    for (auto fParam : node->getParams()) {
        fParam->accept(this);
    }
    node->getBlock()->accept(this);
}

void AtomASTVisitor::visit(BasicType* node) {}

void AtomASTVisitor::visit(ArrayType* node) { node->getBaseDataType()->accept(this); }

void AtomASTVisitor::visit(PointerType* node) { node->getBaseDataType()->accept(this); }

void AtomASTVisitor::visit(Variable* node) {
    node->getDataType()->accept(this);
    if (node->getInitValue()) {
        node->getInitValue()->accept(this);
    }
}

void AtomASTVisitor::visit(ConstVal* node) {}

void AtomASTVisitor::visit(VarRef* node) {}

void AtomASTVisitor::visit(IndexedRef* node) {
    for (auto dimension : node->getDimensions()) {
        dimension->accept(this);
    }
}

void AtomASTVisitor::visit(NestedExpression* node) {
    for (auto expr : node->getElements()) {
        expr->accept(this);
    }
}

void AtomASTVisitor::visit(UnaryExpression* node) { node->getOperand()->accept(this); }

void AtomASTVisitor::visit(BinaryExpression* node) {
    node->getLeft()->accept(this);
    node->getRight()->accept(this);
}

void AtomASTVisitor::visit(FunctionCall* node) {
    for (auto rParam : node->getParams()) {
        rParam->accept(this);
    }
}

void AtomASTVisitor::visit(Block* node) {
    for (auto element : node->getElements()) {
        element->accept(this);
    }
}

void AtomASTVisitor::visit(AssignStatement* node) {
    node->getLval()->accept(this);
    node->getRval()->accept(this);
}

void AtomASTVisitor::visit(IfStatement* node) {
    node->getCond()->accept(this);
    node->getStmt()->accept(this);
    if (node->getElseStmt()) {
        node->getElseStmt()->accept(this);
    }
}

void AtomASTVisitor::visit(ElseStatement* node) { node->getStmt()->accept(this); }

void AtomASTVisitor::visit(WhileStatement* node) {
    node->getCond()->accept(this);
    node->getStmt()->accept(this);
}

void AtomASTVisitor::visit(BreakStatement*) {}

void AtomASTVisitor::visit(ContinueStatement*) {}

void AtomASTVisitor::visit(ReturnStatement* node) {
    if (node->getExpr()) {
        node->getExpr()->accept(this);
    }
}

void AtomASTVisitor::visit(OtherStatement* node) { node->getExpr()->accept(this); }
}  // namespace ATC
