#ifndef ATC_AST_VISITOR_H
#define ATC_AST_VISITOR_H

namespace ATC {

class TreeNode;
class CompUnit;
class VarDecl;
class FunctionDef;
class BasicType;
class ArrayType;
class PointerType;
class Variable;
class ConstVal;
class VarRef;
class IndexedRef;
class NestedExpression;
class UnaryExpression;
class BinaryExpression;
class FunctionCall;
class Block;
class AssignStatement;
class IfStatement;
class ElseStatement;
class WhileStatement;
class BreakStatement;
class ContinueStatement;
class ReturnStatement;
class OtherStatement;

class ASTVisitor {
public:
    virtual void visit(TreeNode*);

    virtual void visit(CompUnit*);

    virtual void visit(VarDecl*);

    virtual void visit(FunctionDef*);

    virtual void visit(BasicType*);

    virtual void visit(ArrayType*);

    virtual void visit(PointerType*);

    virtual void visit(Variable*);

    virtual void visit(ConstVal*);

    virtual void visit(VarRef*);

    virtual void visit(IndexedRef*);

    virtual void visit(NestedExpression*);

    virtual void visit(UnaryExpression*);

    virtual void visit(BinaryExpression*);

    virtual void visit(FunctionCall*);

    virtual void visit(Block*);

    virtual void visit(AssignStatement*);

    virtual void visit(IfStatement*);

    virtual void visit(ElseStatement*);

    virtual void visit(WhileStatement*);

    virtual void visit(BreakStatement*);

    virtual void visit(ContinueStatement*);

    virtual void visit(ReturnStatement*);

    virtual void visit(OtherStatement*);
};
}  // namespace ATC

#endif