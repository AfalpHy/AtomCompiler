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
    for (auto param : node->getParams()) {
        DataType *dataType = param->getVariables()[0]->getDataType();
        funcType._params.push_back(convertToAtomType(dataType));
    }
    _currentFunction = new Function(_currentModule, funcType, node->getName());
    _currentModule->addFunction(_currentFunction);
    _currentBasicBlock = new BasicBlock(_currentFunction, "entry");
    _currentFunction->insertBB(_currentBasicBlock);
    allocForScopeVars(node->getScope());
    int i = 0;
    for (auto param : node->getParams()) {
        // the decl of formal param is the only one
        Variable *var = param->getVariables()[0];
        auto arg = _currentFunction->getParams()[i++];
        createStore(arg, var->getAtomAddr());
    }
    AtomASTVisitor::visit(node);
}

void IRBuilder::visit(Variable *node) {
    Type *basicType = convertToAtomType(node->getBasicType());
    if (node->isGlobal()) {
        // create global variable
        if (auto initValue = node->getInitValue()) {
            initValue->accept(this);
        } else {
            if (node->getDataType()->getClassId() == ID_ARRAY_TYPE) {
                auto arrayValue = new ArrayValue(basicType);
                arrayValue->addElement({static_cast<ArrayType *>(node->getDataType())->getTotalSize(), {}});
                _value = arrayValue;
            } else {
                if (basicType == Type::getInt32Ty()) {
                    _value = new ConstantInt(0);
                } else {
                    _value = new ConstantFloat(0);
                }
            }
        }

        GloabalVariable *globalVar = new GloabalVariable(basicType, node->getName());
        globalVar->setInitialValue(_value);
        _currentModule->addGlobalVariable(globalVar);

        node->setAtomAddr(globalVar);
    } else {
        if (auto initValue = node->getInitValue()) {
            initValue->accept(this);
        }
    }
}

void IRBuilder::visit(ConstVal *node) {
    if (node->getBasicType() == BasicType::INT) {
        _value = new ConstantInt(node->getIntValue());
    } else {
        _value = new ConstantFloat(node->getFloatValue());
    }
    _value->setBelong(_currentFunction);
}

void IRBuilder::visit(VarRef *node) {
    _value = createUnaryInst(INST_LOAD, node->getVariable()->getAtomAddr(), node->getName());
}

// void IRBuilder::visit(IndexedRef *node) {}

void IRBuilder::visit(NestedExpression *node) {
    static int deep = 0;
    static int index;
    // dimensions of definded variable
    static std::vector<int> dimensions;

    static ArrayValue *arrayValue = nullptr;
    static std::vector<Value *> element;
    static int zeroNum;
    if (deep == 0) {
        index = 0;
        dimensions.clear();
        element.clear();
        zeroNum = 0;
        assert(node->getParent()->getClassId() == ID_VARIABLE);
        auto var = (Variable *)node->getParent();
        assert(var->getDataType()->getClassId() == ID_ARRAY_TYPE);
        arrayValue = new ArrayValue(convertToAtomType(var->getBasicType()));
        dimensions = ((ArrayType *)var->getDataType())->getDimensions();
    }

    // The maximum number of elements in a nested expression
    int maxSize = 1;
    for (int i = deep; i < dimensions.size(); i++) {
        maxSize *= dimensions[i];
    }

    deep++;
    const auto &elements = node->getElements();
    // get one value when there are excess elements in a scalar initializer or
    // when there are too many braces around a scalar initializer.
    if (index % maxSize || deep > dimensions.size()) {
        if (!elements.empty()) {
            if (elements[0]->getClassId() == ID_NESTED_EXPRESSION) {
                elements[0]->accept(this);
            } else {
                elements[0]->accept(this);
                element.push_back(_value);
                index++;
            }
        }
    } else {
        int targetIndex = index + maxSize;
        for (size_t i = 0; i < elements.size(); i++) {
            if (elements[i]->getClassId() == ID_NESTED_EXPRESSION) {
                elements[i]->accept(this);
            } else {
                elements[i]->accept(this);
                element.push_back(_value);
                index++;
            }
            // ignore the remaining elements
            if (index == targetIndex) {
                break;
            }
        }
        if (index != targetIndex) {
            if (!element.empty()) {
                if (zeroNum) {
                    arrayValue->addElement({zeroNum, {}});
                    zeroNum = 0;
                }
                arrayValue->addElement({element.size(), element});
                element.clear();
            }
            zeroNum += targetIndex - index;
        }
        index = targetIndex;
    }
    deep--;
    if (deep == 0) {
        if (zeroNum) {
            arrayValue->addElement({zeroNum, {}});
        }
        _value = arrayValue;
    }
}

// void IRBuilder::visit(UnaryExpression *node) {}

void IRBuilder::visit(BinaryExpression *node) {
    node->getLeft()->accept(this);
    Value *left = _value;
    node->getRight()->accept(this);
    Value *right = _value;

    switch (node->getOperator()) {
        case PLUS:
            _value = createBinaryInst(INST_ADD, left, right);
            break;
        case MINUS:
            _value = createBinaryInst(INST_SUB, left, right);
            break;
        case MUL:
            _value = createBinaryInst(INST_MUL, left, right);
            break;
        case DIV:
            _value = createBinaryInst(INST_DIV, left, right);
            break;
        case MOD:
            _value = createBinaryInst(INST_MOD, left, right);
            break;
        case LT:
            _value = createBinaryInst(INST_LT, left, right);
            break;
        case GT:
            _value = createBinaryInst(INST_GT, left, right);
            break;
        case LE:
            _value = createBinaryInst(INST_LE, left, right);
            break;
        case GE:
            _value = createBinaryInst(INST_GE, left, right);
            break;
        case EQ:
            _value = createBinaryInst(INST_EQ, left, right);
            break;
        case NE:
            _value = createBinaryInst(INST_NE, left, right);
            break;
        default:
            assert(false && "should not reach here");
            break;
    }
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
    result->setBelong(_currentFunction);
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
    result->setBelong(_currentFunction);
    return result;
}

Value *IRBuilder::createBinaryInst(InstType type, Value *operand1, Value *operand2, const std::string &resultName) {
    Instruction *inst = new BinaryInst(type, operand1, operand2, resultName);
    _currentBasicBlock->addInstruction(inst);
    Value *result = inst->getResult();
    result->setBelong(_currentFunction);
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

void IRBuilder::allocForScopeVars(Scope *currentScope) {
    for (const auto &[name, var] : currentScope->getVarMap()) {
        Value *addr = createAlloc(convertToAtomType(var->getDataType()), name);
        var->setAtomAddr(addr);
    }
    for (auto child : currentScope->getChildren()) {
        allocForScopeVars(child);
    }
}

}  // namespace AtomIR
}  // namespace ATC