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
    virtual void visit(TreeNode*);

    virtual void visit(CompUnit*);

    virtual void visit(Decl*);

    virtual void visit(FunctionDef*);

    virtual void visit(DataType*) = 0;

    virtual void visit(Variable*);

    virtual void visit(ConstVal*) = 0;

    virtual void visit(VarRef*);

    virtual void visit(ArrayExpression*);

    virtual void visit(UnaryExpression*);

    virtual void visit(BinaryExpression*);

    virtual void visit(FunctionCall*);

    virtual void visit(Block*);

    virtual void visit(AssignStatement*);

    virtual void visit(IfStatement*);

    virtual void visit(ElseStatement*);

    virtual void visit(WhileStatement*);

    virtual void visit(ReturnStatement*);

    virtual void visit(OtherStatement*);
};
}  // namespace ATC

#endif