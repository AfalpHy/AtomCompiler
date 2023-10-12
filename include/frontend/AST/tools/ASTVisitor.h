#ifndef AST_VISITOR_H
#define AST_VISITOR_H

namespace ATC {

class TreeNode;
class CompUnit;
class Decl;
class FuncDef;
class Variable;
class Expression;
class ConstVal;
class VarRef;
class UnaryExpression;
class BinaryExpression;
class Statement;
class Block;
class AssignStatement;
class IfStatement;
class ReturnStatement;

class ASTVisitor {
public:
    virtual void visit(TreeNode*) = 0;

    virtual void visit(CompUnit*) = 0;

    virtual void visit(Decl*) = 0;

    virtual void visit(FuncDef*) = 0;

    virtual void visit(Variable*) = 0;

    // virtual void visit(Expression*) = 0;

    virtual void visit(ConstVal*) = 0;

    virtual void visit(VarRef*) = 0;

    virtual void visit(UnaryExpression*) = 0;

    virtual void visit(BinaryExpression*) = 0;

    // virtual void visit(Statement*) = 0;

    virtual void visit(Block*) = 0;

    virtual void visit(AssignStatement*) = 0;

    virtual void visit(IfStatement*) = 0;

    virtual void visit(ReturnStatement*) = 0;
};
}  // namespace ATC

#endif