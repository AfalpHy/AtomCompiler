#pragma once

#include "AST/ASTVisitor.h"
#include "Module.h"

namespace ATC {

class DataType;
class Scope;

namespace AtomIR {

class IRBuilder : public ASTVisitor {
public:
    IRBuilder();
    ~IRBuilder();

    virtual void visit(CompUnit *) override;

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

public:
    Module *getCurrentModule() { return _currentModule; }

    void dumpIR(const std::string& filePath);

private:
    Value *createAlloc(Type *allocType, const std::string &resultName = "");

    void createStore(Value *value, Value *dest);

    Value *createFunctionCall(const FunctionType &functionType, const std::string &funcName,
                              const std::vector<Value *> &params, const std::string &resultName = "");

    Value *createGEP(Value *ptr, const std::vector<Value *> &indexes, const std::string &resultName = "");

    Value *createBitCast(Value *ptr, Type *destTy);

    void createRet(Value *retValue);

    Value *createUnaryInst(int type, Value *operand, const std::string &resultName = "");

    Value *createBinaryInst(int type, Value *operand1, Value *operand2, const std::string &resultName = "");

    void createJump(BasicBlock *targetBB);

    void createCondJump(int type, BasicBlock *trueBB, BasicBlock *falseBB, Value *operand1, Value *operand2);

    void createCondJumpForValue(BasicBlock *trueBB, BasicBlock *falseBB, Value *value);

    Type *convertToAtomType(int type);

    Type *convertToAtomType(DataType *dataType);

    Value *castToDestTyIfNeed(Value *value, Type *destTy);

    Value *getIndexedRefAddress(IndexedRef *indexedRef);

private:
    Module *_currentModule;
    Function *_currentFunction;
    BasicBlock *_currentBasicBlock;
    Value *_value;

    Type *_voidTy;
    Type *_int32Ty;
    Type *_int32PtrTy;
    Type *_floatTy;
    Type *_floatPtrTy;

    ConstantInt *_int32Zero;
    ConstantFloat *_floatZero;
    ConstantInt *_int32One;
    ConstantFloat *_floatOne;

    BasicBlock *_trueBB = nullptr;
    BasicBlock *_falseBB = nullptr;

    // for break/continue statement
    BasicBlock *_condBB = nullptr;
    BasicBlock *_afterBB = nullptr;

    std::unordered_map<Variable *, Value *> _var2addr;
    std::unordered_map<std::string, FunctionType *> _funcName2funcType;
};
}  // namespace AtomIR
}  // namespace ATC
