#ifndef SEMANTIC_CHECKER_H
#define SEMANTIC_CHECKER_H

#include <iostream>

#include "ASTVisitor.h"

namespace ATC {
class SemanticChecker : public ASTVisitor {
public:
    // virtual void visit(TreeNode*) override;

    // virtual void visit(CompUnit*) override;

    // virtual void visit(Decl*) override;

    // virtual void visit(FunctionDef*) override;

    // virtual void visit(DataType*) override;

    // virtual void visit(Variable*) override;

    // virtual void visit(ConstVal*) override;

    virtual void visit(VarRef*) override;

    // virtual void visit(ArrayExpression*) override;

    // virtual void visit(UnaryExpression*) override;

    // virtual void visit(BinaryExpression*) override;

    virtual void visit(FunctionCall*) override;

    // virtual void visit(Block*) override;

    // virtual void visit(AssignStatement*) override;

    // virtual void visit(IfStatement*) override;

    // virtual void visit(ElseStatement*) override;

    // virtual void visit(WhileStatement*) override;

    // virtual void visit(ReturnStatement*) override;

    // virtual void visit(OtherStatement*) override;
};
}  // namespace ATC

#endif