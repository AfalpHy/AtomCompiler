#ifndef DUMP_AST_VISITOR_H
#define DUMP_AST_VISITOR_H

#include <iostream>

#include "ASTVisitor.h"

namespace ATC {
class DumpASTVisitor : public ASTVisitor {
public:
    virtual void visit(TreeNode*) override;

    virtual void visit(CompUnit*) override;

    virtual void visit(Decl*) override;

    virtual void visit(FuncDef*) override;

    virtual void visit(DataType*) override;

    virtual void visit(Variable*) override;

    // virtual void visit(Expression*) override;

    virtual void visit(ConstVal*) override;

    virtual void visit(VarRef*) override;

    virtual void visit(UnaryExpression*) override;

    virtual void visit(BinaryExpression*) override;

    // virtual void visit(Statement*) override;

    virtual void visit(Block*) override;

    virtual void visit(AssignStatement*) override;

    virtual void visit(IfStatement*) override;

    virtual void visit(ElseStatement*) override;

    virtual void visit(ReturnStatement*) override;

private:
    void printNode(TreeNode* node);

private:
    int _indent = 0;
};
}  // namespace ATC

#endif