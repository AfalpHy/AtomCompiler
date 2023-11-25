#ifndef ATOMIR_BUILDER_H
#define ATOMIR_BUILDER_H

#include <string>

#include "AST/AtomASTVisitor.h"
#include "Instruction.h"
namespace ATC {

class DataType;

namespace AtomIR {

class Module;
struct FunctionType;
class Function;
class BasicBlock;
class Value;
class Type;

class IRBuilder : public AtomASTVisitor {
public:
    IRBuilder();
    ~IRBuilder();

    virtual void visit(CompUnit *) override;

    virtual void visit(FunctionDef *) override;

    virtual void visit(Variable *) override;

    virtual void visit(ConstVal *) override;

    virtual void visit(VarRef *) override;

    // virtual void visit(IndexedRef *) override;

    // virtual void visit(NestedExpression *) override;

    // virtual void visit(UnaryExpression *) override;

    virtual void visit(BinaryExpression *) override;

    // virtual void visit(FunctionCall *) override;

    // virtual void visit(Block *) override;

    virtual void visit(AssignStatement *) override;

    // virtual void visit(IfStatement *) override;

    // virtual void visit(WhileStatement *) override;

    // virtual void visit(BreakStatement *) override;

    // virtual void visit(ContinueStatement *) override;

    // virtual void visit(ReturnStatement *) override;
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

private:
    Module *_currentModule;
    Function *_currentFunction;
    BasicBlock *_currentBasicBlock;
    Value *_value;
};
}  // namespace AtomIR
}  // namespace ATC

#endif