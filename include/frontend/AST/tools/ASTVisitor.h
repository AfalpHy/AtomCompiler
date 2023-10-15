#ifndef AST_VISITOR_H
#define AST_VISITOR_H

namespace ATC {

class TreeNode;
class CompUnit;
class Decl;
class FunctionDef;
class DataType;
class Variable;
class Expression;
class ConstVal;
class VarRef;
class ArrayExpression;
class UnaryExpression;
class BinaryExpression;
class FunctionCall;
class Statement;
class Block;
class AssignStatement;
class IfStatement;
class ElseStatement;
class WhileStatement;
class ReturnStatement;
class OtherStatement;
class ASTVisitor {
public:
    virtual void visit(TreeNode*) = 0;

    virtual void visit(CompUnit*) = 0;

    virtual void visit(Decl*) = 0;

    virtual void visit(FunctionDef*) = 0;

    virtual void visit(DataType*) = 0;

    virtual void visit(Variable*) = 0;

    virtual void visit(ConstVal*) = 0;

    virtual void visit(VarRef*) = 0;

    virtual void visit(ArrayExpression*) = 0;

    virtual void visit(UnaryExpression*) = 0;

    virtual void visit(BinaryExpression*) = 0;

    virtual void visit(FunctionCall*) = 0;

    virtual void visit(Block*) = 0;

    virtual void visit(AssignStatement*) = 0;

    virtual void visit(IfStatement*) = 0;

    virtual void visit(ElseStatement*) = 0;

    virtual void visit(WhileStatement*) = 0;

    virtual void visit(ReturnStatement*) = 0;

    virtual void visit(OtherStatement*) = 0;
};
}  // namespace ATC

#endif