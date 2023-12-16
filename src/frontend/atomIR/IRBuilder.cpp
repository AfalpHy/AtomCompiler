#include "atomIR/IRBuilder.h"

#include "AST/CompUnit.h"
#include "AST/Decl.h"
#include "AST/Expression.h"
#include "AST/FunctionDef.h"
#include "AST/Scope.h"
#include "AST/Statement.h"
#include "AST/Variable.h"

namespace ATC {

namespace AtomIR {
IRBuilder::IRBuilder() {
    _voidTy = Type::getVoidTy();
    _int32Ty = Type::getInt32Ty();
    _floatTy = Type::getFloatTy();
    _int32PtrTy = _int32Ty->getPointerTy();
    _floatPtrTy = _floatTy->getPointerTy();
    _int32Zero = ConstantInt::get(0);
    _floatZero = ConstantFloat::get(0);
    _int32One = ConstantInt::get(1);
    _floatOne = ConstantFloat::get(1);
}

IRBuilder::~IRBuilder() { _currentModule->dump(); }

void IRBuilder::visit(CompUnit *node) {
    _currentModule = new Module(node->getName());
    ASTVisitor::visit(node);
}

void IRBuilder::visit(FunctionDef *node) {
    FunctionType funcType;
    funcType._ret = convertToAtomType(node->getRetType());
    for (auto param : node->getParams()) {
        DataType *dataType = param->getVariables()[0]->getDataType();
        funcType._params.push_back(convertToAtomType(dataType));
    }
    _currentFunction = new Function(_currentModule, funcType, node->getName());
    _currentBasicBlock = new BasicBlock(_currentFunction, "init");

    int i = 0;
    for (auto param : node->getParams()) {
        param->accept(this);
        // the decl of formal param is the only one
        Variable *var = param->getVariables()[0];
        auto arg = _currentFunction->getParams()[i++];
        createStore(arg, _var2addr[var]);
    }
    _functonDef2function.insert({node, _currentFunction});

    auto entry = new BasicBlock(_currentFunction, "entry");
    createJump(entry);
    _currentBasicBlock = entry;

    ASTVisitor::visit(node);
}

void IRBuilder::visit(Variable *node) {
    Type *basicType = convertToAtomType(node->getBasicType());
    if (node->isGlobal()) {
        // create global variable
        if (auto initValue = node->getInitValue()) {
            initValue->accept(this);
            if (node->getDataType()->getClassId() != ID_ARRAY_TYPE) {
                _value = castToDestTyIfNeed(_value, basicType);
            }
        } else {
            if (node->getDataType()->getClassId() == ID_ARRAY_TYPE) {
                int totalSize = static_cast<ATC::ArrayType *>(node->getDataType())->getTotalSize();
                auto arrayValue = new ArrayValue(ArrayType::get(basicType, totalSize));
                arrayValue->addElement({totalSize, {}});
                _value = arrayValue;
            } else {
                if (basicType == Type::getInt32Ty()) {
                    _value = ConstantInt::get(0);
                } else {
                    _value = ConstantFloat::get(0);
                }
            }
        }

        GloabalVariable *globalVar = new GloabalVariable(_value->getType(), node->getName());
        globalVar->setInitialValue(_value);
        _currentModule->addGlobalVariable(globalVar);

        _var2addr.insert({node, globalVar});
    } else {
        Value *addr = createAlloc(convertToAtomType(node->getDataType()), node->getName());
        _var2addr.insert({node, addr});
        if (auto initValue = node->getInitValue()) {
            initValue->accept(this);
            if (initValue->getClassId() == ID_NESTED_EXPRESSION) {
                ATC::ArrayType *arrayType = static_cast<ATC::ArrayType *>(node->getDataType());
                FunctionType funcType;
                funcType._ret = _voidTy;
                funcType._params.push_back(_voidTy->getPointerTy());
                funcType._params.push_back(_int32Ty);
                createFunctionCall(
                    funcType, "memset",
                    {castToDestTyIfNeed(addr, _voidTy->getPointerTy()), ConstantInt::get(arrayType->getTotalSize())});

                auto dimension = arrayType->getDimensions();
                auto elementSize = arrayType->getElementSize();
                ArrayValue *arrayValue = (ArrayValue *)_value;
                auto elements = arrayValue->getElements();
                int i = 0;
                for (auto element : elements) {
                    if (element.second.empty()) {
                        i += element.first;
                    } else {
                        for (auto elementValue : element.second) {
                            createStore(elementValue, createGEP(addr, {_int32Zero, ConstantInt::get(i++)}));
                        }
                    }
                }
            } else {
                _value = castToDestTyIfNeed(_value, basicType);
                createStore(_value, addr);
            }
        }
    }
}

void IRBuilder::visit(ConstVal *node) {
    if (node->getBasicType() == BasicType::INT) {
        _value = ConstantInt::get(node->getIntValue());
    } else {
        _value = ConstantFloat::get(node->getFloatValue());
    }
}

void IRBuilder::visit(VarRef *node) {
    if (node->isConst()) {
        if (ExpressionHandle::isIntExpr(node)) {
            _value = ConstantInt::get(ExpressionHandle::evaluateConstExpr(node));
        } else {
            _value = ConstantFloat::get(ExpressionHandle::evaluateConstExpr(node));
        }
        return;
    }
    auto addr = _var2addr[node->getVariable()];
    if (node->getVariable()->getDataType()->getClassId() == ID_ARRAY_TYPE) {
        // cast the array to pointer,
        // int a[10]; 'a' is treated as a pointer when used as a function argument
        _value = addr;
    } else {
        _value = createUnaryInst(UnaryInst::INST_LOAD, addr, node->getName());
    }
}

void IRBuilder::visit(IndexedRef *node) {
    auto addr = getIndexedRefAddress(node);
    Variable *var = node->getVariable();
    ATC::ArrayType *arrayType;
    if (var->getDataType()->getClassId() == ID_POINTER_TYPE) {
        ATC::PointerType *varType = (ATC::PointerType *)var->getDataType();
        arrayType = (ATC::ArrayType *)varType->getBaseDataType();
        if (arrayType->getDimensions().size() + 1 > node->getDimensions().size()) {
            _value = addr;
            return;
        }
    } else {
        arrayType = (ATC::ArrayType *)var->getDataType();
        if (arrayType->getDimensions().size() > node->getDimensions().size()) {
            _value = addr;
            return;
        }
    }
    _value = createUnaryInst(UnaryInst::INST_LOAD, addr);
}

void IRBuilder::visit(NestedExpression *node) {
    static int deep = 0;
    static int index;
    // dimensions of definded variable
    static std::vector<int> dimensions;

    static ArrayValue *arrayValue = nullptr;
    static std::vector<Value *> element;
    static int zeroNum;
    static Type *basicType;
    if (deep == 0) {
        index = 0;
        dimensions.clear();
        element.clear();
        zeroNum = 0;
        assert(node->getParent()->getClassId() == ID_VARIABLE);
        auto var = (Variable *)node->getParent();
        assert(var->getDataType()->getClassId() == ID_ARRAY_TYPE);
        ATC::ArrayType *varType = (ATC::ArrayType *)var->getDataType();
        basicType = convertToAtomType(var->getBasicType());
        arrayValue = new ArrayValue(ArrayType::get(basicType, varType->getTotalSize()));
        dimensions = varType->getDimensions();
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
                element.push_back(castToDestTyIfNeed(_value, basicType));
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
                element.push_back(castToDestTyIfNeed(_value, basicType));
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

void IRBuilder::visit(UnaryExpression *node) {
    if (node->isConst()) {
        if (ExpressionHandle::isIntExpr(node)) {
            _value = ConstantInt::get(ExpressionHandle::evaluateConstExpr(node));
        } else {
            _value = ConstantFloat::get(ExpressionHandle::evaluateConstExpr(node));
        }
        return;
    }
    node->getOperand()->accept(this);
    if (node->getOperator() == MINUS) {
        if (_value->getType() == _floatTy) {
            _value = createBinaryInst(BinaryInst::INST_SUB, _floatZero, _value);
        } else if (_value->getType() == _int32Ty) {
            _value = createBinaryInst(BinaryInst::INST_SUB, _int32Zero, _value);
        } else {
            _value = createBinaryInst(BinaryInst::INST_SUB, _int32Zero, castToDestTyIfNeed(_value, _int32Ty));
        }
    } else if (node->getOperator() == NOT) {
        if (_value->getType() == _floatTy) {
            _value = createBinaryInst(BinaryInst::INST_NE, _floatZero, _value);
        } else if (_value->getType() == _int32Ty) {
            _value = createBinaryInst(BinaryInst::INST_NE, _int32Zero, _value);
        } else {
            _value = createBinaryInst(BinaryInst::INST_NE, _int32One, castToDestTyIfNeed(_value, _int32Ty));
        }
    }
}

void IRBuilder::visit(BinaryExpression *node) {
    if (node->isConst()) {
        if (ExpressionHandle::isIntExpr(node)) {
            _value = ConstantInt::get(ExpressionHandle::evaluateConstExpr(node));
        } else {
            _value = ConstantFloat::get(ExpressionHandle::evaluateConstExpr(node));
        }
        return;
    }
    if (node->getOperator() == AND || node->getOperator() == OR) {
        // calculate the value of short circuit expr, not for condition
        bool forValue = ExpressionHandle::isForValue(node);
        Value *valueAddr = nullptr;
        BasicBlock *saveTrueBB = nullptr;
        BasicBlock *saveFalseBB = nullptr;
        if (forValue) {
            saveTrueBB = _trueBB;
            saveFalseBB = _falseBB;
            _trueBB = new BasicBlock(_currentFunction, "valueOneBB");

            _falseBB = new BasicBlock(_currentFunction, "valueZeroBB");

            valueAddr = createAlloc(_int32Ty);
        }

        BasicBlock *rhsCondBB = new BasicBlock(_currentFunction, "rhsCondBB");

        if (node->getOperator() == AND) {
            auto tmpTrueBB = _trueBB;
            _trueBB = rhsCondBB;
            node->getLeft()->accept(this);
            createCondJump(CondJumpInst::INST_JNE, rhsCondBB, _falseBB, _value,
                           castToDestTyIfNeed(_int32Zero, _value->getType()));
            _trueBB = tmpTrueBB;
        } else {
            auto tmpFalseBB = _falseBB;
            _falseBB = rhsCondBB;
            node->getLeft()->accept(this);
            createCondJump(CondJumpInst::INST_JNE, rhsCondBB, _falseBB, _value,
                           castToDestTyIfNeed(_int32Zero, _value->getType()));
            _falseBB = tmpFalseBB;
        }

        _currentBasicBlock = rhsCondBB;
        node->getRight()->accept(this);

        if (forValue) {
            createCondJump(CondJumpInst::INST_JNE, _trueBB, _falseBB, _value,
                           castToDestTyIfNeed(_int32Zero, _value->getType()));

            BasicBlock *afterCalcShortCircuitBB = new BasicBlock(_currentFunction, "afterCalcShortCircuitBB");

            _currentBasicBlock = _trueBB;
            createStore(_int32One, valueAddr);
            createJump(afterCalcShortCircuitBB);

            _currentBasicBlock = _falseBB;
            createStore(_int32Zero, valueAddr);
            createJump(afterCalcShortCircuitBB);

            _currentBasicBlock = afterCalcShortCircuitBB;
            _value = createUnaryInst(UnaryInst::INST_LOAD, valueAddr);

            _trueBB = saveTrueBB;
            _falseBB = saveFalseBB;
        }
        return;
    }
    node->getLeft()->accept(this);
    Value *left = _value;
    node->getRight()->accept(this);
    Value *right = _value;

    // make the left expr and right expr has the same type
    if (left->getType() == _floatTy || right->getType() == _floatTy) {
        left = castToDestTyIfNeed(left, _floatTy);
        right = castToDestTyIfNeed(right, _floatTy);
    } else {
        left = castToDestTyIfNeed(left, _int32Ty);
        right = castToDestTyIfNeed(right, _int32Ty);
    }

    switch (node->getOperator()) {
        case PLUS:
            _value = createBinaryInst(BinaryInst::INST_ADD, left, right);
            break;
        case MINUS:
            _value = createBinaryInst(BinaryInst::INST_SUB, left, right);
            break;
        case MUL:
            _value = createBinaryInst(BinaryInst::INST_MUL, left, right);
            break;
        case DIV:
            _value = createBinaryInst(BinaryInst::INST_DIV, left, right);
            break;
        case MOD:
            _value = createBinaryInst(BinaryInst::INST_MOD, left, right);
            break;
        case LT: {
            if (ExpressionHandle::isForValue(node)) {
                _value = createBinaryInst(BinaryInst::INST_LT, left, right);
            } else {
                createCondJump(CondJumpInst::INST_JLT, _trueBB, _falseBB, left, right);
            }
            break;
        }
        case GT: {
            if (ExpressionHandle::isForValue(node)) {
                _value = createBinaryInst(BinaryInst::INST_GT, left, right);
            } else {
                createCondJump(CondJumpInst::INST_JGT, _trueBB, _falseBB, left, right);
            }
            break;
        }
        case LE: {
            if (ExpressionHandle::isForValue(node)) {
                _value = createBinaryInst(BinaryInst::INST_LE, left, right);
            } else {
                createCondJump(CondJumpInst::INST_JLE, _trueBB, _falseBB, left, right);
            }
            break;
        }
        case GE: {
            if (ExpressionHandle::isForValue(node)) {
                _value = createBinaryInst(BinaryInst::INST_GE, left, right);
            } else {
                createCondJump(CondJumpInst::INST_JGE, _trueBB, _falseBB, left, right);
            }
            break;
        }
        case EQ: {
            if (ExpressionHandle::isForValue(node)) {
                _value = createBinaryInst(BinaryInst::INST_EQ, left, right);
            } else {
                createCondJump(CondJumpInst::INST_JEQ, _trueBB, _falseBB, left, right);
            }
            break;
        }
        case NE: {
            if (ExpressionHandle::isForValue(node)) {
                _value = createBinaryInst(BinaryInst::INST_NE, left, right);
            } else {
                createCondJump(CondJumpInst::INST_JNE, _trueBB, _falseBB, left, right);
            }
            break;
        }
        default:
            assert(false && "should not reach here");
            break;
    }
}

void IRBuilder::visit(FunctionCall *node) {
    std::vector<Value *> params;
    Function *function = nullptr;
    if (node->getFunctionDef()) {
        function = _functonDef2function[node->getFunctionDef()];
    } else {
        /// TODO:
    }
    int i = 0;
    for (auto rParam : node->getParams()) {
        rParam->accept(this);
        _value = castToDestTyIfNeed(_value, function->getParams()[i++]->getType());
        params.push_back(_value);
    }
    _value = createFunctionCall(function->getFunctionType(), function->getName(), params);
}

void IRBuilder::visit(Block *node) {
    for (auto element : node->getElements()) {
        element->accept(this);
        // skip the statements following the jump statement
        if (_currentBasicBlock->isHasBr()) {
            return;
        }
    }
}

void IRBuilder::visit(AssignStatement *node) {
    Value *addr = nullptr;
    if (node->getLval()->getClassId() == ID_VAR_REF) {
        addr = _var2addr[static_cast<VarRef *>(node->getLval())->getVariable()];
    } else {
        addr = getIndexedRefAddress((IndexedRef *)node->getLval());
    }
    node->getRval()->accept(this);
    _value = castToDestTyIfNeed(_value, static_cast<PointerType *>(addr->getType())->getBaseType());
    _currentBasicBlock->addInstruction(new StoreInst(_value, addr));
}

void IRBuilder::visit(IfStatement *node) {
    auto ifBB = new BasicBlock(_currentFunction, "ifBB");

    auto afterIfBB = new BasicBlock(_currentFunction, "afterIfBB");

    _trueBB = ifBB;
    if (node->getElseStmt()) {
        auto elseBB = new BasicBlock(_currentFunction, "elseBB");

        _falseBB = elseBB;
        node->getCond()->accept(this);
        createCondJump(CondJumpInst::INST_JNE, _trueBB, _falseBB, _value,
                       castToDestTyIfNeed(_int32Zero, _value->getType()));

        _currentBasicBlock = elseBB;
        node->getElseStmt()->accept(this);
        createJump(afterIfBB);
    } else {
        _falseBB = afterIfBB;
        node->getCond()->accept(this);
        createCondJump(CondJumpInst::INST_JNE, _trueBB, _falseBB, _value,
                       castToDestTyIfNeed(_int32Zero, _value->getType()));
    }

    _currentBasicBlock = ifBB;
    node->getStmt()->accept(this);
    createJump(afterIfBB);

    _currentBasicBlock = afterIfBB;

    _trueBB = nullptr;
    _falseBB = nullptr;
}

void IRBuilder::visit(WhileStatement *node) {
    auto condBB = new BasicBlock(_currentFunction, "condBB");

    auto whileBB = new BasicBlock(_currentFunction, "whileBB");

    auto afterWhileBB = new BasicBlock(_currentFunction, "afterWhileBB");

    createJump(condBB);
    _currentBasicBlock = condBB;
    _trueBB = whileBB;
    _falseBB = afterWhileBB;
    node->getCond()->accept(this);
    createCondJump(CondJumpInst::INST_JNE, _trueBB, _falseBB, _value,
                   castToDestTyIfNeed(_int32Zero, _value->getType()));

    auto tmpCondBB = _condBB;
    auto tmpAfterBB = _afterBB;
    _condBB = condBB;
    _afterBB = afterWhileBB;
    _currentBasicBlock = whileBB;
    node->getStmt()->accept(this);
    createJump(condBB);
    _condBB = tmpCondBB;
    _afterBB = tmpAfterBB;

    _currentBasicBlock = afterWhileBB;

    _trueBB = nullptr;
    _falseBB = nullptr;
}

void IRBuilder::visit(BreakStatement *node) { createJump(_afterBB); }

void IRBuilder::visit(ContinueStatement *node) { createJump(_condBB); }

void IRBuilder::visit(ReturnStatement *node) {
    if (node->getExpr()) {
        node->getExpr()->accept(this);
        createRet(_value);
    } else {
        createRet(nullptr);
    }
}

Value *IRBuilder::createAlloc(Type *allocType, const std::string &resultName) {
    auto entryBB = _currentFunction->getBasicBlocks().front();
    Instruction *inst = new AllocInst(allocType, resultName);
    entryBB->getInstructionList().push_front(inst);
    Value *result = inst->getResult();
    result->setBelong(_currentFunction);
    _currentFunction->setNeedUpdateName(true);
    return result;
}

void IRBuilder::createStore(Value *value, Value *dest) {
    Instruction *inst = new StoreInst(value, dest);
    _currentBasicBlock->addInstruction(inst);
}

Value *IRBuilder::createFunctionCall(const FunctionType &functionType, const std::string &funcName,
                                     const std::vector<Value *> &params, const std::string &resultName) {
    Instruction *inst = new FunctionCallInst(functionType, funcName, params, resultName);
    _currentBasicBlock->addInstruction(inst);
    Value *result = inst->getResult();
    if (result) {
        result->setBelong(_currentFunction);
    }
    return result;
}

Value *IRBuilder::createGEP(Value *ptr, const std::vector<Value *> &indexes, const std::string &resultName) {
    Instruction *inst = new GetElementPtrInst(ptr, indexes, resultName);
    _currentBasicBlock->addInstruction(inst);
    Value *result = inst->getResult();
    result->setBelong(_currentFunction);
    return result;
}

Value *IRBuilder::createBitCast(Value *ptr, Type *destTy) {
    if (ptr->getType() == destTy) {
        return ptr;
    }
    Instruction *inst = new BitCastInst(ptr, destTy);
    _currentBasicBlock->addInstruction(inst);
    Value *result = inst->getResult();
    result->setBelong(_currentFunction);
    return result;
}

void IRBuilder::createRet(Value *retValue) {
    if (_currentBasicBlock->isHasBr()) {
        return;
    }
    Instruction *inst = new ReturnInst(retValue);
    _currentBasicBlock->addInstruction(inst);
    _currentBasicBlock->setHasBr();
}

Value *IRBuilder::createUnaryInst(int type, Value *operand, const std::string &resultName) {
    Instruction *inst = new UnaryInst(type, operand, resultName);
    _currentBasicBlock->addInstruction(inst);
    Value *result = inst->getResult();
    result->setBelong(_currentFunction);
    return result;
}

Value *IRBuilder::createBinaryInst(int type, Value *operand1, Value *operand2, const std::string &resultName) {
    Instruction *inst = new BinaryInst(type, operand1, operand2, resultName);
    _currentBasicBlock->addInstruction(inst);
    Value *result = inst->getResult();
    result->setBelong(_currentFunction);
    return result;
}

void IRBuilder::createJump(BasicBlock *targetBB) {
    if (_currentBasicBlock->isHasBr()) {
        return;
    }
    Instruction *inst = new JumpInst(targetBB);
    _currentBasicBlock->addInstruction(inst);
    _currentBasicBlock->addSuccessor(targetBB);
    targetBB->addPredecessor(_currentBasicBlock);
    _currentBasicBlock->setHasBr();
}

void IRBuilder::createCondJump(int type, BasicBlock *trueBB, BasicBlock *falseBB, Value *operand1, Value *operand2) {
    if (_currentBasicBlock->isHasBr()) {
        return;
    }
    Instruction *inst = new CondJumpInst(type, trueBB, falseBB, operand1, operand2);
    _currentBasicBlock->addInstruction(inst);
    _currentBasicBlock->addSuccessor(trueBB);
    _currentBasicBlock->addSuccessor(falseBB);
    trueBB->addPredecessor(_currentBasicBlock);
    falseBB->addPredecessor(_currentBasicBlock);
    _currentBasicBlock->setHasBr();
}

Type *IRBuilder::convertToAtomType(int basicType) {
    switch (basicType) {
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
    } else if (dataType->getClassId() == ID_ARRAY_TYPE) {
        return ArrayType::get(convertToAtomType(dataType->getBasicType()),
                              static_cast<ATC::ArrayType *>(dataType)->getTotalSize());
    } else {
        return convertToAtomType(dataType->getBasicType());
    }
}

Value *IRBuilder::castToDestTyIfNeed(Value *value, Type *destTy) {
    if (destTy->isPointerType()) {
        return createBitCast(value, destTy);
    }
    if (destTy == _floatTy) {
        if (value->isConst()) {
            return ConstantFloat::get(std::stof(value->getValueStr()));
        } else if (value->getType() == _int32Ty) {
            return createUnaryInst(UnaryInst::INST_ITOF, value);
        }
    } else if (destTy == _int32Ty) {
        if (value->isConst()) {
            return ConstantInt::get(std::stoi(value->getValueStr()));
        } else if (value->getType() == _floatTy) {
            return createUnaryInst(UnaryInst::INST_FTOI, value);
        }
    }
    return value;
}

Value *IRBuilder::getIndexedRefAddress(IndexedRef *indexedRef) {
    Variable *var = indexedRef->getVariable();
    Value *addr = _var2addr[var];

    assert(addr->getType()->isPointerType());
    PointerType *addrType = (PointerType *)addr->getType();
    const auto &dimension = indexedRef->getDimensions();
    std::vector<int> elementSize;

    if (var->getDataType()->getClassId() == ID_POINTER_TYPE) {
        addr = createUnaryInst(UnaryInst::INST_LOAD, addr);
        int i = 0;
        dimension[i++]->accept(this);
        addr = createGEP(addr, {_value});

        ATC::PointerType *varType = (ATC::PointerType *)var->getDataType();
        if (varType->getBaseDataType()->getClassId() == ID_ARRAY_TYPE) {
            elementSize = static_cast<ATC::ArrayType *>(varType->getBaseDataType())->getElementSize();
            for (; i != dimension.size(); i++) {
                dimension[i]->accept(this);
                _value = createBinaryInst(BinaryInst::INST_MUL, _value, ConstantInt::get(elementSize[i - 1]));
            }
            if (i > 1) {
                return createGEP(addr, {_int32Zero, _value});
            }
        }
        return addr;
    }
    elementSize = static_cast<ATC::ArrayType *>(var->getDataType())->getElementSize();
    int i = 0;
    dimension[i]->accept(this);
    _value = createBinaryInst(BinaryInst::INST_MUL, _value, ConstantInt::get(elementSize[i]));

    for (i = 1; i != dimension.size(); i++) {
        dimension[i]->accept(this);
        auto tmp = createBinaryInst(BinaryInst::INST_MUL, _value, ConstantInt::get(elementSize[i]));
        _value = createBinaryInst(BinaryInst::INST_ADD, _value, tmp);
    }
    return createGEP(addr, {_int32Zero, _value});
}

}  // namespace AtomIR
}  // namespace ATC