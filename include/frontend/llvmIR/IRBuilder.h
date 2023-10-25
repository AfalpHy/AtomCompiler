#ifndef IR_BUILDER_H
#define IR_BUILDER_H

#include "AST/tools/ASTVisitor.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"

namespace ATC {

class Scope;

class IRBuilder : public ASTVisitor {
public:
    IRBuilder();
    ~IRBuilder();

    // virtual void visit(TreeNode *) override;

    // virtual void visit(CompUnit *) override;

    // virtual void visit(Decl *) override;

    virtual void visit(FunctionDef *) override;

    // virtual void visit(DataType *) override;

    virtual void visit(Variable *) override;

    virtual void visit(ConstVal *) override;

    virtual void visit(VarRef *) override;

    virtual void visit(ArrayExpression *) override;

    virtual void visit(UnaryExpression *) override;

    virtual void visit(BinaryExpression *) override;

    virtual void visit(FunctionCall *) override;

    // virtual void visit(Block *) override;

    virtual void visit(AssignStatement *) override;

    virtual void visit(IfStatement *) override;

    // virtual void visit(ElseStatement *) override;

    virtual void visit(WhileStatement *) override;

    virtual void visit(ReturnStatement *) override;

    // virtual void visit(OtherStatement *) override;

private:
    llvm::Type *convertToLLVMType(int type);
    llvm::Type *convertToLLVMType(DataType *dataType);
    void allocForScopeVars(Scope *currentScope);
    llvm::Value *convertToDestTy(llvm::Value *value, llvm::Type *destTy);

    llvm::Module *_module;
    llvm::IRBuilder<> *_theIRBuilder;
    llvm::Type *_voidTy;
    llvm::Type *_int1Ty;
    llvm::Type *_int32Ty;
    llvm::Type *_int32PtrTy;
    llvm::Type *_floatTy;
    llvm::Type *_floatPtrTy;

    llvm::Constant *_int1Zero;
    llvm::Constant *_int1One;
    llvm::Constant *_int32Zero;
    llvm::Constant *_floatZero;
    llvm::Constant *_int32One;
    llvm::Constant *_floatOne;

    // 保存遍历表达式节点后的求出的值
    llvm::Value *_value;
    llvm::BasicBlock *_trueBB;
    llvm::BasicBlock *_falseBB;
    llvm::Function *_currentFunction;
};
}  // namespace ATC
#endif