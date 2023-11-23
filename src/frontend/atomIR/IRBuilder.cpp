#include "atomIR/IRBuilder.h"

#include "AST/CompUnit.h"
#include "AST/Decl.h"
#include "AST/Expression.h"
#include "AST/FunctionDef.h"
#include "AST/Scope.h"
#include "AST/Statement.h"
#include "AST/Variable.h"
#include "atomIR/IRDumper.h"
#include "atomIR/Module.h"
#include "atomIR/Value.h"
namespace ATC {

namespace AtomIR {
IRBuilder::IRBuilder() {}

IRBuilder::~IRBuilder() {
    IRDumper irDumper;
    irDumper.dump(_currentModule);
}

void IRBuilder::visit(CompUnit *node) {
    _currentModule = new Module(node->getName());
    AtomASTVisitor::visit(node);
}

void IRBuilder::visit(FunctionDef *node) {
    _currentFunction = new Function(_currentModule, node->getName());
    _currentModule->addFunction(_currentFunction);
    _currentBasicBlock = new BasicBlock(_currentFunction, "entry");
    _currentFunction->insertBB(_currentBasicBlock);
    AtomASTVisitor::visit(node);
}

void IRBuilder::visit(Variable *node) { node->setAtomAddr(createAlloc(Type::getInt32Ty(), node->getName())); }

void IRBuilder::visit(ConstVal *node) {}

void IRBuilder::visit(VarRef *node) {
    _value = createUnaryInst(INST_LOAD, node->getVariable()->getAtomAddr(), node->getName());
}

// void IRBuilder::visit(IndexedRef *node) {}

// void IRBuilder::visit(NestedExpression *node) {}

// void IRBuilder::visit(UnaryExpression *node) {}

void IRBuilder::visit(BinaryExpression *node) {
    node->getLeft()->accept(this);
    Value *left = _value;
    node->getRight()->accept(this);
    Value *right = _value;
    switch (node->getOperator()) {
        case PLUS: {
            Instruction *addInst = new BinaryInst(INST_ADD, left, right, "res");
            _value = addInst->getResult();
            _currentBasicBlock->addInstruction(addInst);
            break;
        }
        default:
            break;
    }
}

// void IRBuilder::visit(FunctionCall *node) {}

// void IRBuilder::visit(Block *node) {}

void IRBuilder::visit(AssignStatement *node) {}

// void IRBuilder::visit(IfStatement *node) {}

// void IRBuilder::visit(WhileStatement *node) {}

// void IRBuilder::visit(BreakStatement *node) {}

// void IRBuilder::visit(ContinueStatement *node) {}

// void IRBuilder::visit(ReturnStatement *node) {}

Value *IRBuilder::createAlloc(Type *allocType, const std::string &resultName) {
    Instruction *inst = new AllocInst(allocType, resultName);
    _currentBasicBlock->addInstruction(inst);
    return inst->getResult();
}

void IRBuilder::createStore(Value *value, Value *dest) {
    Instruction *inst = new StoreInst(value, dest);
    _currentBasicBlock->addInstruction(inst);
}

void IRBuilder::createRet(Value *retValue) {
    Instruction *inst = new ReturnInst(retValue);
    _currentBasicBlock->addInstruction(inst);
}

Value *IRBuilder::createUnaryInst(InstType type, Value *operand, const std::string &resultName) {
    Instruction *inst = new UnaryInst(type, operand, resultName);
    _currentBasicBlock->addInstruction(inst);
    return inst->getResult();
}

Value *IRBuilder::createBinaryInst(InstType type, Value *operand1, Value *operand2, const std::string &resultName) {
    Instruction *inst = new BinaryInst(type, operand1, operand2, resultName);
    _currentBasicBlock->addInstruction(inst);
    return inst->getResult();
}

void IRBuilder::createJump(BasicBlock *targetBB) {
    Instruction *inst = new JumpInst(targetBB);
    _currentBasicBlock->addInstruction(inst);
}

void IRBuilder::createCondJump(InstType type, BasicBlock *trueBB, BasicBlock *falseBB, Value *operand1,
                               Value *operand2) {
    Instruction *inst = new CondJumpInst(type, trueBB, falseBB, operand1, operand2);
    _currentBasicBlock->addInstruction(inst);
}

}  // namespace AtomIR
}  // namespace ATC