#ifndef IR_BUILDER_H
#define IR_BUILDER_H

#include <map>

#include "AST/tools/ASTVisitor.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
namespace ATC {

class Scope;
class DataType;

class IRBuilder : public ASTVisitor {
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

    virtual void visit(ReturnStatement *) override;

private:
    llvm::Type *convertToLLVMType(int type);
    llvm::Type *convertToLLVMType(DataType *dataType);
    void allocForScopeVars(Scope *currentScope);
    llvm::Value *convertToDestTy(llvm::Value *value, llvm::Type *destTy);
    // check if there is a return statement in the current basic block, and create the br if not
    void checkAndCreateBr(llvm::BasicBlock *destBlk);
    void checkAndCreateCondBr(llvm::Value *value, llvm::BasicBlock *trueBlk, llvm::BasicBlock *falseBlck);

    llvm::Value *getIndexedRefAddress(IndexedRef *varRef);
    llvm::Value *convertNestedValuesToConstant(const std::vector<int> &dimensions, int deep, int begin, llvm::Type *basicType);

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

    std::set<llvm::BasicBlock *> _retBlk;
    std::map<int, llvm::Value *> _nestedExpressionValues;
};
}  // namespace ATC
#endif