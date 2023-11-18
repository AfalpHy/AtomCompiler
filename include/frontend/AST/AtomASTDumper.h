#ifndef ATOM_AST_DUMPER_H
#define ATOM_AST_DUMPER_H

#include <iostream>

#include "AtomASTVisitor.h"

namespace ATC {
class AtomASTDumper : public AtomASTVisitor {
public:
    virtual void visit(TreeNode*) override;

    virtual void visit(CompUnit*) override;

    virtual void visit(VarDecl*) override;

    virtual void visit(FunctionDef*) override;

    virtual void visit(BasicType*) override;

    virtual void visit(ArrayType*) override;

    virtual void visit(PointerType*) override;

    virtual void visit(Variable*) override;

    virtual void visit(ConstVal*) override;

    virtual void visit(VarRef*) override;

    virtual void visit(IndexedRef*) override;

    virtual void visit(NestedExpression*) override;

    virtual void visit(UnaryExpression*) override;

    virtual void visit(BinaryExpression*) override;

    virtual void visit(FunctionCall*) override;

    virtual void visit(Block*) override;

    virtual void visit(AssignStatement*) override;

    virtual void visit(IfStatement*) override;

    virtual void visit(ElseStatement*) override;

    virtual void visit(WhileStatement*) override;

    virtual void visit(ReturnStatement*) override;

    virtual void visit(OtherStatement*) override;

private:
    void printNode(TreeNode* node, bool newLine = true);

private:
    int _indent = 0;
};
}  // namespace ATC

#endif