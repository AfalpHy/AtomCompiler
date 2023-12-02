#ifndef ATC_IR_BUILDER_H
#define ATC_IR_BUILDER_H

#include <string>

#include "AST/ASTVisitor.h"
#include "Instruction.h"
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

    // virtual void visit(IndexedRef *) override;

    virtual void visit(NestedExpression *) override;

    // virtual void visit(UnaryExpression *) override;

    virtual void visit(BinaryExpression *) override;

    // virtual void visit(FunctionCall *) override;

    // virtual void visit(Block *) override;

    virtual void visit(AssignStatement *) override;

    // virtual void visit(IfStatement *) override;

    // virtual void visit(WhileStatement *) override;

    // virtual void visit(BreakStatement *) override;

    // virtual void visit(ContinueStatement *) override;

    virtual void visit(ReturnStatement *) override;

private:
    Value *createAlloc(Type *allocType, const std::string &resultName = "");

    void createStore(Value *value, Value *dest);

    void createFunctionCall(FunctionType functionType, const std::string &funcName, const std::vector<Value *> &params,
                            const std::string &resultName = "");

    void createRet(Value *retValue);

    Value *createUnaryInst(InstType type, Value *operand, const std::string &resultName = "");

    Value *createBinaryInst(InstType type, Value *operand1, Value *operand2, const std::string &resultName = "");

    void createJump(BasicBlock *targetBB);

    void createCondJump(InstType type, BasicBlock *trueBB, BasicBlock *falseBB, Value *operand1, Value *operand2);

    Type *convertToAtomType(int type);

    Type *convertToAtomType(DataType *dataType);

    void allocForScopeVars(Scope *currentScope);

private:
    Module *_currentModule;
    Function *_currentFunction;
    BasicBlock *_currentBasicBlock;
    Value *_value;

    Type *_voidTy;
    Type *_int1Ty;
    Type *_int32Ty;
    Type *_int32PtrTy;
    Type *_floatTy;
    Type *_floatPtrTy;

    ConstantInt *_int32Zero;
    ConstantFloat *_floatZero;
    ConstantInt *_int32One;
    ConstantFloat *_floatOne;
};
}  // namespace AtomIR
}  // namespace ATC

#endif