#ifndef ATC_LLVM_IR_BUILDER_H
#define ATC_LLVM_IR_BUILDER_H

#include <map>

#include "AST/ASTVisitor.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"

namespace ATC {

class Scope;
class DataType;

namespace LLVMIR {
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

    virtual void visit(BreakStatement *) override;

    virtual void visit(ContinueStatement *) override;

    virtual void visit(ReturnStatement *) override;

    void dumpLL(std::string path) {
        std::error_code EC;
        llvm::raw_fd_ostream outputFile(path, EC);
        _module->print(outputFile, nullptr);
    }

private:
    llvm::Type *convertToLLVMType(int type);
    llvm::Type *convertToLLVMType(DataType *dataType);
    void allocForScopeVars(Scope *currentScope);
    llvm::Value *castToDestTyIfNeed(llvm::Value *value, llvm::Type *destTy);
    // check if there is a return statement in the current basic block, and create the br if not
    void checkAndCreateBr(llvm::BasicBlock *destBlk);
    void checkAndCreateCondBr(llvm::Value *value, llvm::BasicBlock *trueBlk, llvm::BasicBlock *falseBlck);

    llvm::Value *getIndexedRefAddress(IndexedRef *varRef);
    llvm::Value *convertNestedValuesToConstant(const std::vector<int> &dimensions, int deep, int begin,
                                               llvm::Type *basicType);

    llvm::Module *_module;
    llvm::IRBuilder<> *_theIRBuilder;
    llvm::Type *_voidTy;
    llvm::Type *_int1Ty;
    llvm::Type *_int32Ty;
    llvm::Type *_int32PtrTy;
    llvm::Type *_floatTy;
    llvm::Type *_floatPtrTy;

    llvm::Constant *_int32Zero;
    llvm::Constant *_floatZero;
    llvm::Constant *_int32One;
    llvm::Constant *_floatOne;

    // keep the value of expression
    llvm::Value *_value = nullptr;

    llvm::BasicBlock *_trueBB = nullptr;
    llvm::BasicBlock *_falseBB = nullptr;

    // for break/continue statement
    llvm::BasicBlock *_condBB = nullptr;
    llvm::BasicBlock *_afterBB = nullptr;

    llvm::Function *_currentFunction = nullptr;

    std::set<llvm::BasicBlock *> _hasBrOrRetBlk;
    std::map<int, llvm::Value *> _nestedExpressionValues;
    std::map<std::string, llvm::Function *> _definedElseWhere;
};
}  // namespace LLVMIR
}  // namespace ATC
#endif