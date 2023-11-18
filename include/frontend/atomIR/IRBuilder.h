#ifndef IR_BUILDER_H
#define IR_BUILDER_H

#include "AST/AtomASTVisitor.h"

namespace ATC {

namespace AtomIR {
class IRBuilder : public AtomASTVisitor {
public:
    IRBuilder();
    ~IRBuilder();

    virtual void visit(FunctionDef *) override;

    virtual void visit(Variable *) override;

    virtual void visit(ConstVal *) override;

    virtual void visit(VarRef *) override;

    virtual void visit(IndexedRef *) override;

    virtual void visit(NestedExpression *) override;

    virtual void visit(UnaryExpression *) override;

    virtual void visit(BinaryExpression *) override;

    virtual void visit(FunctionCall *) override;

    virtual void visit(Block *) override;

    virtual void visit(AssignStatement *) override;

    virtual void visit(IfStatement *) override;

    virtual void visit(WhileStatement *) override;

    virtual void visit(BreakStatement *) override;

    virtual void visit(ContinueStatement *) override;

    virtual void visit(ReturnStatement *) override;
};
}  // namespace AtomIR
}  // namespace ATC

#endif