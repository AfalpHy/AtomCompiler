#include "atomIR/IRBuilder.h"

#include "AST/CompUnit.h"
#include "AST/Decl.h"
#include "AST/Expression.h"
#include "AST/FunctionDef.h"
#include "AST/Scope.h"
#include "AST/Statement.h"
#include "AST/Variable.h"
#include "atomIR/Module.h"
#include "atomIR/Value.h"
namespace ATC {

namespace AtomIR {
IRBuilder::IRBuilder() {}

IRBuilder::~IRBuilder() { _currentModule->dump(); }

void IRBuilder::visit(CompUnit *node) {
    _currentModule = new Module(node->getName());
    AtomASTVisitor::visit(node);
}

void IRBuilder::visit(FunctionDef *node) {
    FunctionType funcType;
    funcType._ret = Type::getInt32Ty();
    // for (auto param : node->getParams()) {
    // }
    _currentFunction = new Function(_currentModule, funcType, node->getName());
    _currentModule->addFunction(_currentFunction);
    _currentBasicBlock = new BasicBlock(_currentFunction, "entry");
    _currentFunction->insertBB(_currentBasicBlock);
    AtomASTVisitor::visit(node);
}

void IRBuilder::visit(Variable *node) { node->setAtomAddr(createAlloc(Type::getInt32Ty(), node->getName())); }

void IRBuilder::visit(ConstVal *node) {
    if (node->getBasicType() == BasicType::INT) {
        _value = new Value(Type::getInt32Ty(), node->getIntValue());
    } else {
        _value = new Value(Type::getFloatTy(), node->getFloatValue());
    }
    _value->setBelongFunction(_currentFunction);
}

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
    Instruction *binaryInst = nullptr;
    switch (node->getOperator()) {
        case PLUS:
            binaryInst = new BinaryInst(INST_ADD, left, right);
            break;
        case MINUS:
            binaryInst = new BinaryInst(INST_SUB, left, right);
            break;
        case MUL:
            binaryInst = new BinaryInst(INST_MUL, left, right);
            break;
        case DIV:
            binaryInst = new BinaryInst(INST_DIV, left, right);
            break;
        case MOD:
            binaryInst = new BinaryInst(INST_MOD, left, right);
            break;
        case LT:
            binaryInst = new BinaryInst(INST_LT, left, right);
            break;
        case GT:
            binaryInst = new BinaryInst(INST_GT, left, right);
            break;
        case LE:
            binaryInst = new BinaryInst(INST_LE, left, right);
            break;
        case GE:
            binaryInst = new BinaryInst(INST_GE, left, right);
            break;
        case EQ:
            binaryInst = new BinaryInst(INST_EQ, left, right);
            break;
        case NE:
            binaryInst = new BinaryInst(INST_NE, left, right);
            break;
        default:
            assert(false && "should not reach here");
            break;
    }
    _value = binaryInst->getResult();
    _currentBasicBlock->addInstruction(binaryInst);
}

// void IRBuilder::visit(FunctionCall *node) {}

// void IRBuilder::visit(Block *node) {}

void IRBuilder::visit(AssignStatement *node) {
    Value *addr = nullptr;
    if (node->getLval()->getClassId() == ID_VAR_REF) {
        addr = static_cast<VarRef *>(node->getLval())->getVariable()->getAtomAddr();
    } else {
        // addr = getIndexedRefAddress((IndexedRef *)node->getLval());
    }
    node->getRval()->accept(this);
    _currentBasicBlock->addInstruction(new StoreInst(_value, addr));
}

// void IRBuilder::visit(IfStatement *node) {}

// void IRBuilder::visit(WhileStatement *node) {}

// void IRBuilder::visit(BreakStatement *node) {}

// void IRBuilder::visit(ContinueStatement *node) {}

void IRBuilder::visit(ReturnStatement *node) {
    if (node->getExpr()) {
        node->getExpr()->accept(this);
        _currentBasicBlock->addInstruction(new ReturnInst(_value));
    } else {
        _currentBasicBlock->addInstruction(new ReturnInst());
    }
}

Value *IRBuilder::createAlloc(Type *allocType, const std::string &resultName) {
    Instruction *inst = new AllocInst(allocType, resultName);
    _currentBasicBlock->addInstruction(inst);
    Value *result = inst->getResult();
    result->setBelongFunction(_currentFunction);
    return result;
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
    Value *result = inst->getResult();
    result->setBelongFunction(_currentFunction);
    return result;
}

Value *IRBuilder::createBinaryInst(InstType type, Value *operand1, Value *operand2, const std::string &resultName) {
    Instruction *inst = new BinaryInst(type, operand1, operand2, resultName);
    _currentBasicBlock->addInstruction(inst);
    Value *result = inst->getResult();
    result->setBelongFunction(_currentFunction);
    return result;
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

Type *IRBuilder::convertToAtomType(int basicType) {
    switch (basicType) {
        case BasicType::BOOL:
            return Type::getInt1Ty();
        case BasicType::INT:
            return Type::getInt32Ty();
        case BasicType::FLOAT:
            return Type::getFloatTy();
        case BasicType::VOID:
            return Type::getVoidTy();
        default:
            assert(false && "should not reach here");
    }
}

Type *IRBuilder::convertToAtomType(DataType *dataType) {
    if (dataType->getClassId() == ID_POINTER_TYPE) {
        return PointerType::get(convertToAtomType(dataType->getBaseDataType()));
    } else {
        return convertToAtomType(dataType->getBasicType());
    }
}

}  // namespace AtomIR
}  // namespace ATC