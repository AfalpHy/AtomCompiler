#include "IR/IRBuilder.h"

#include "../CmdOption.h"
#include "AST/CompUnit.h"
#include "AST/Expression.h"
#include "AST/Function.h"
#include "AST/Scope.h"
#include "AST/Statement.h"
#include "AST/Variable.h"

namespace ATC {

namespace IR {
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

    if (Sy) {
        auto funcTy = FunctionType::get(_int32Ty, {}, false);
        _funcName2funcType["getint"] = funcTy;
        _funcName2funcType["getch"] = funcTy;

        funcTy = FunctionType::get(_floatTy, {}, false);
        _funcName2funcType["getfloat"] = funcTy;

        funcTy = FunctionType::get(_int32Ty, {_int32PtrTy}, false);
        _funcName2funcType["getarray"] = funcTy;

        funcTy = FunctionType::get(_int32Ty, {_floatPtrTy}, false);
        _funcName2funcType["getfarray"] = funcTy;

        funcTy = FunctionType::get(_voidTy, {_int32Ty, _int32PtrTy}, false);
        _funcName2funcType["putarray"] = funcTy;

        funcTy = FunctionType::get(_voidTy, {_floatTy}, false);
        _funcName2funcType["putfloat"] = funcTy;

        funcTy = FunctionType::get(_voidTy, {_int32Ty, _floatPtrTy}, false);
        _funcName2funcType["putfarray"] = funcTy;

        funcTy = FunctionType::get(_voidTy, {_int32PtrTy}, true);
        _funcName2funcType["putf"] = funcTy;

        funcTy = FunctionType::get(_voidTy, {}, false);
        _funcName2funcType["before_main"] = funcTy;
        _funcName2funcType["after_main"] = funcTy;

        funcTy = FunctionType::get(_voidTy, {_int32Ty}, false);
        _funcName2funcType["putint"] = funcTy;
        _funcName2funcType["putch"] = funcTy;
        _funcName2funcType["_sysy_starttime"] = funcTy;
        _funcName2funcType["_sysy_stoptime"] = funcTy;
    }
}

IRBuilder::~IRBuilder() {}

void IRBuilder::visit(CompUnit *node) {
    _currentModule = new Module(node->getName());
    ASTVisitor::visit(node);
}

void IRBuilder::visit(FunctionDecl *node) {
    std::vector<Type *> paramTypeVec;
    for (auto param : node->getParams()) {
        paramTypeVec.push_back(convertToIRType(param->getVariables()[0]->getDataType()));
    }
    _funcName2funcType[node->getName()] = FunctionType::get(convertToIRType(node->getRetType()), paramTypeVec, false);
}

void IRBuilder::visit(FunctionDef *node) {
    auto functionDecl = node->getFunctionDecl();
    std::vector<Type *> params;
    for (auto param : functionDecl->getParams()) {
        DataType *dataType = param->getVariables()[0]->getDataType();
        params.push_back(convertToIRType(dataType));
    }

    auto funcType = FunctionType::get(convertToIRType(functionDecl->getRetType()), params, false);
    _funcName2funcType.insert({functionDecl->getName(), funcType});

    _currentFunction = new Function(_currentModule, *funcType, functionDecl->getName());
    _currentBasicBlock = new BasicBlock(_currentFunction, "entryBB");

    AllocInst::AllocatedIntParamNum = 0;
    AllocInst::AllocatedFloatParamNum = 0;
    AllocInst::AllocForParam = true;
    int i = 0;
    for (auto param : functionDecl->getParams()) {
        param->accept(this);
        // the decl of formal param is the only one
        Variable *var = param->getVariables()[0];
        auto arg = _currentFunction->getParams()[i++];
        createStore(arg, _var2addr[var]);
    }
    AllocInst::AllocForParam = false;

    auto beginBB = new BasicBlock(_currentFunction, "beginBB");
    createJump(beginBB);
    _currentBasicBlock = beginBB;

    node->getBlock()->accept(this);

    // if the function didn't execute return before, than return the default value
    if (!_currentBasicBlock->isHasBr()) {
        if (functionDecl->getRetType()->getBasicType() == BasicType::INT) {
            createRet(_int32Zero);
        } else if (functionDecl->getRetType()->getBasicType() == BasicType::FLOAT) {
            createRet(_floatZero);
        } else {
            createRet(nullptr);
        }
    }
    maskDeadInst();
}

void IRBuilder::visit(Variable *node) {
    Type *basicType = convertToIRType(node->getBasicType());
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
                if (basicType == _int32Ty) {
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
        Value *addr = createAlloc(convertToIRType(node->getDataType()), node->getName());
        _var2addr.insert({node, addr});
        if (auto initValue = node->getInitValue()) {
            if (initValue->getClassId() == ID_NESTED_EXPRESSION) {
                ATC::ArrayType *arrayType = static_cast<ATC::ArrayType *>(node->getDataType());
                FunctionType *funcType =
                    FunctionType::get(_voidTy, {_voidTy->getPointerTy(), _int32Ty, _int32Ty}, false);
                createFunctionCall(*funcType, "memset",
                                   {castToDestTyIfNeed(addr, _voidTy->getPointerTy()), ConstantInt::get(0),
                                    ConstantInt::get(arrayType->getTotalSize() * 4)});
                initValue->accept(this);
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
                initValue->accept(this);
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
            _value = ConstantInt::get(ExpressionHandle::evaluateConstIntExpr(node));
        } else {
            _value = ConstantFloat::get(ExpressionHandle::evaluateConstFloatExpr(node));
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
    if (node->isConst()) {
        if (ExpressionHandle::isIntExpr(node)) {
            _value = ConstantInt::get(ExpressionHandle::evaluateConstIntExpr(node));
        } else {
            _value = ConstantFloat::get(ExpressionHandle::evaluateConstFloatExpr(node));
        }
        return;
    }
    auto addr = getIndexedRefAddress(node);
    Variable *var = node->getVariable();
    ATC::ArrayType *arrayType;
    if (var->getDataType()->getClassId() == ID_POINTER_TYPE) {
        ATC::PointerType *varType = (ATC::PointerType *)var->getDataType();
        if (varType->getBaseDataType()->getClassId() == ID_ARRAY_TYPE) {
            arrayType = (ATC::ArrayType *)varType->getBaseDataType();
            if (arrayType->getDimensions().size() + 1 > node->getDimensions().size()) {
                _value = addr;
                return;
            }
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
        auto var = node->getVariable();
        assert(var->getDataType()->getClassId() == ID_ARRAY_TYPE);
        ATC::ArrayType *varType = (ATC::ArrayType *)var->getDataType();
        basicType = convertToIRType(var->getBasicType());
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

        if (!element.empty()) {
            if (zeroNum) {
                arrayValue->addElement({zeroNum, {}});
                zeroNum = 0;
            }
            arrayValue->addElement({element.size(), element});
            element.clear();
        }
        zeroNum += targetIndex - index;

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
            _value = ConstantInt::get(ExpressionHandle::evaluateConstIntExpr(node));
        } else {
            _value = ConstantFloat::get(ExpressionHandle::evaluateConstFloatExpr(node));
        }
        return;
    }
    node->getOperand()->accept(this);
    if (node->getOperator() == MINUS) {
        if (_value->getType() == _floatTy) {
            _value = createBinaryInst(BinaryInst::INST_SUB, _floatZero, _value);
        } else if (_value->getType() == _int32Ty) {
            _value = createBinaryInst(BinaryInst::INST_SUB, _int32Zero, _value);
        }
    } else if (node->getOperator() == NOT) {
        if (_value->getType() == _floatTy) {
            _value = createBinaryInst(BinaryInst::INST_EQ, _floatZero, _value);
        } else if (_value->getType() == _int32Ty) {
            _value = createBinaryInst(BinaryInst::INST_EQ, _int32Zero, _value);
        }
    }
}

void IRBuilder::visit(BinaryExpression *node) {
    if (node->isConst()) {
        if (ExpressionHandle::isIntExpr(node)) {
            _value = ConstantInt::get(ExpressionHandle::evaluateConstIntExpr(node));
        } else {
            _value = ConstantFloat::get(ExpressionHandle::evaluateConstFloatExpr(node));
        }
        return;
    }
    if (node->getOperator() == AND || node->getOperator() == OR) {
        // calculate the value of short circuit expr, not for condition
        bool forValue = node->isForValue();
        Value *valueAddr = nullptr;
        BasicBlock *saveTrueBB = nullptr;
        BasicBlock *saveFalseBB = nullptr;
        if (forValue) {
            saveTrueBB = _trueBB;
            saveFalseBB = _falseBB;
            _trueBB = new BasicBlock(_currentFunction, "valueOneBB");

            _falseBB = new BasicBlock(_currentFunction, "valueZeroBB");

            valueAddr = createAlloc(_int32Ty, "OneOrZero");
        }

        BasicBlock *rhsCondBB = new BasicBlock(_currentFunction, "rhsCondBB");

        if (node->getOperator() == AND) {
            auto tmpTrueBB = _trueBB;
            _trueBB = rhsCondBB;
            node->getLeft()->accept(this);
            createCondJumpForValue(rhsCondBB, _falseBB, _value);
            _trueBB = tmpTrueBB;
        } else {
            auto tmpFalseBB = _falseBB;
            _falseBB = rhsCondBB;
            node->getLeft()->accept(this);
            createCondJumpForValue(_trueBB, rhsCondBB, _value);
            _falseBB = tmpFalseBB;
        }

        _currentBasicBlock = rhsCondBB;
        node->getRight()->accept(this);

        if (forValue) {
            createCondJumpForValue(_trueBB, _falseBB, _value);

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

    Value *left;
    Value *right;

    // When the left expression is neither a binary operation nor a function call,
    // the right expression is evaluated first.
    // This is particularly useful when the right expression involves a binary operation or a function call.
    if (node->getLeft()->getClassId() != ID_BINARY_EXPRESSION && node->getLeft()->getClassId() != ID_FUNCTION_CALL) {
        node->getRight()->accept(this);
        right = _value;
        node->getLeft()->accept(this);
        left = _value;
    } else {
        node->getLeft()->accept(this);
        left = _value;
        node->getRight()->accept(this);
        right = _value;
    }

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
            if (node->isForValue()) {
                _value = createBinaryInst(BinaryInst::INST_LT, left, right);
            } else {
                createCondJump(CondJumpInst::INST_JLT, _trueBB, _falseBB, left, right);
            }
            break;
        }
        case GT: {
            if (node->isForValue()) {
                _value = createBinaryInst(BinaryInst::INST_GT, left, right);
            } else {
                createCondJump(CondJumpInst::INST_JGT, _trueBB, _falseBB, left, right);
            }
            break;
        }
        case LE: {
            if (node->isForValue()) {
                _value = createBinaryInst(BinaryInst::INST_LE, left, right);
            } else {
                createCondJump(CondJumpInst::INST_JLE, _trueBB, _falseBB, left, right);
            }
            break;
        }
        case GE: {
            if (node->isForValue()) {
                _value = createBinaryInst(BinaryInst::INST_GE, left, right);
            } else {
                createCondJump(CondJumpInst::INST_JGE, _trueBB, _falseBB, left, right);
            }
            break;
        }
        case EQ: {
            if (node->isForValue()) {
                _value = createBinaryInst(BinaryInst::INST_EQ, left, right);
            } else {
                createCondJump(CondJumpInst::INST_JEQ, _trueBB, _falseBB, left, right);
            }
            break;
        }
        case NE: {
            if (node->isForValue()) {
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
    if (Sy && (node->getName() == "starttime" || node->getName() == "stoptime")) {
        std::string funName = "_sysy_" + node->getName();
        _value = createFunctionCall(*_funcName2funcType[funName], funName,
                                    {ConstantInt::get(node->getPosition()._leftLine)});
        return;
    }
    std::vector<Value *> params;
    FunctionType *funcTy = nullptr;
    funcTy = _funcName2funcType[node->getName()];
    int i = 0;
    for (auto rParam : node->getParams()) {
        rParam->accept(this);
        _value = castToDestTyIfNeed(_value, funcTy->_params[i++]);
        params.push_back(_value);
    }
    _value = createFunctionCall(*funcTy, node->getName(), params);
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
    node->getRval()->accept(this);
    auto rVal = _value;
    Value *addr = nullptr;
    if (node->getLval()->getClassId() == ID_VAR_REF) {
        addr = _var2addr[static_cast<VarRef *>(node->getLval())->getVariable()];
    } else {
        addr = getIndexedRefAddress((IndexedRef *)node->getLval());
    }

    rVal = castToDestTyIfNeed(rVal, static_cast<PointerType *>(addr->getType())->getBaseType());
    _currentBasicBlock->addInstruction(new StoreInst(rVal, addr));
}

void IRBuilder::visit(IfStatement *node) {
    auto ifBB = new BasicBlock(_currentFunction, "ifBB");

    auto afterIfBB = new BasicBlock(_currentFunction, "afterIfBB");

    _trueBB = ifBB;
    if (node->getElseStmt()) {
        auto elseBB = new BasicBlock(_currentFunction, "elseBB");

        _falseBB = elseBB;
        node->getCond()->accept(this);
        createCondJumpForValue(_trueBB, _falseBB, _value);

        _currentBasicBlock = elseBB;
        node->getElseStmt()->accept(this);
        createJump(afterIfBB);
    } else {
        _falseBB = afterIfBB;
        node->getCond()->accept(this);
        createCondJumpForValue(_trueBB, _falseBB, _value);
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
    createCondJumpForValue(_trueBB, _falseBB, _value);

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
        createRet(castToDestTyIfNeed(_value, _currentFunction->getFunctionType()._ret));
    } else {
        createRet(nullptr);
    }
}

Value *IRBuilder::createAlloc(Type *allocType, const std::string &resultName) {
    auto entryBB = _currentFunction->getBasicBlocks().front();
    auto &instList = entryBB->getInstructionList();
    Instruction *inst = new AllocInst(allocType, resultName);
    if (_currentFunction->isCurAllocIterInit()) {
        auto tmp = _currentFunction->getCurAllocIter();
        tmp++;
        instList.insert(tmp, inst);
        _currentFunction->getCurAllocIter()++;
    } else {
        instList.push_front(inst);
        _currentFunction->getCurAllocIter() = instList.begin();
        _currentFunction->setCurAllocIterInit();
    }
    Value *result = inst->getResult();
    result->setBelongAndInsertName(_currentFunction);
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
    _currentFunction->setHasFunctionCall(true);
    Value *result = inst->getResult();
    if (result) {
        result->setBelongAndInsertName(_currentFunction);
    }
    return result;
}

Value *IRBuilder::createGEP(Value *ptr, const std::vector<Value *> &indexes, const std::string &resultName) {
    Instruction *inst = new GetElementPtrInst(ptr, indexes, resultName);
    _currentBasicBlock->addInstruction(inst);
    Value *result = inst->getResult();
    result->setBelongAndInsertName(_currentFunction);
    return result;
}

Value *IRBuilder::createBitCast(Value *ptr, Type *destTy) {
    if (ptr->getType() == destTy) {
        return ptr;
    }
    Instruction *inst = new BitCastInst(ptr, destTy);
    _currentBasicBlock->addInstruction(inst);
    Value *result = inst->getResult();
    result->setBelongAndInsertName(_currentFunction);
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
    result->setBelongAndInsertName(_currentFunction);
    return result;
}

Value *IRBuilder::createBinaryInst(int type, Value *operand1, Value *operand2, const std::string &resultName) {
    Instruction *inst = new BinaryInst(type, operand1, operand2, resultName);
    _currentBasicBlock->addInstruction(inst);
    Value *result = inst->getResult();
    result->setBelongAndInsertName(_currentFunction);
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

void IRBuilder::createCondJumpForValue(BasicBlock *trueBB, BasicBlock *falseBB, Value *value) {
    if (value->isConst()) {
        auto constValue = (Constant *)value;
        if (constValue->isInt()) {
            auto constInt = (ConstantInt *)value;
            if (constInt->getConstValue()) {
                createJump(_trueBB);
            } else {
                createJump(_falseBB);
            }
        } else {
            auto constFloat = (ConstantFloat *)value;
            if (constFloat->getConstValue()) {
                createJump(_trueBB);
            } else {
                createJump(_falseBB);
            }
        }
    } else {
        createCondJump(CondJumpInst::INST_JNE, _trueBB, _falseBB, value,
                       castToDestTyIfNeed(_int32Zero, value->getType()));
    }
}

Type *IRBuilder::convertToIRType(int basicType) {
    switch (basicType) {
        case BasicType::INT:
            return _int32Ty;
        case BasicType::FLOAT:
            return _floatTy;
        case BasicType::VOID:
            return _voidTy;
        default:
            assert(false && "should not reach here");
    }
}

Type *IRBuilder::convertToIRType(DataType *dataType) {
    if (dataType->getClassId() == ID_POINTER_TYPE) {
        return PointerType::get(convertToIRType(dataType->getBaseDataType()));
    } else if (dataType->getClassId() == ID_ARRAY_TYPE) {
        return ArrayType::get(convertToIRType(dataType->getBasicType()),
                              static_cast<ATC::ArrayType *>(dataType)->getTotalSize());
    } else {
        return convertToIRType(dataType->getBasicType());
    }
}

Value *IRBuilder::castToDestTyIfNeed(Value *value, Type *destTy) {
    if (destTy->isPointerType()) {
        return createBitCast(value, destTy);
    }
    if (destTy == _floatTy) {
        if (value->getType() == _int32Ty) {
            if (value->isConst()) {
                return ConstantFloat::get(static_cast<ConstantInt *>(value)->getConstValue());
            } else {
                return createUnaryInst(UnaryInst::INST_ITOF, value);
            }
        }
    } else if (destTy == _int32Ty) {
        if (value->getType() == _floatTy) {
            if (value->isConst()) {
                return ConstantInt::get(static_cast<ConstantFloat *>(value)->getConstValue());
            } else {
                return createUnaryInst(UnaryInst::INST_FTOI, value);
            }
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

    int constPart = 0;
    int dimIdx = 0;
    if (var->getDataType()->getClassId() == ID_POINTER_TYPE) {
        addr = createUnaryInst(UnaryInst::INST_LOAD, addr);

        dimension[dimIdx++]->accept(this);
        addr = createGEP(addr, {_value});

        ATC::PointerType *varType = (ATC::PointerType *)var->getDataType();
        if (varType->getBaseDataType()->getClassId() == ID_ARRAY_TYPE) {
            elementSize = static_cast<ATC::ArrayType *>(varType->getBaseDataType())->getElementSize();
        } else {
            return addr;
        }
    } else {
        elementSize = static_cast<ATC::ArrayType *>(var->getDataType())->getElementSize();
    }

    Value *tmp = nullptr;
    int sizeIdx = 0;
    while (dimIdx < dimension.size()) {
        dimension[dimIdx++]->accept(this);
        if (_value->isConst()) {
            constPart += static_cast<ConstantInt *>(_value)->getConstValue() * elementSize[sizeIdx++];
        } else {
            _value = createBinaryInst(BinaryInst::INST_MUL, _value, ConstantInt::get(elementSize[sizeIdx++]));
            tmp = tmp ? createBinaryInst(BinaryInst::INST_ADD, _value, tmp) : _value;
        }
    }

    if (tmp == nullptr) {
        tmp = ConstantInt::get(constPart);
    } else if (constPart) {
        tmp = createBinaryInst(BinaryInst::INST_ADD, ConstantInt::get(constPart), tmp);
    }
    return createGEP(addr, {_int32Zero, tmp});
}

void IRBuilder::maskDeadInst() {
    bool update;
    do {
        update = false;
        std::set<Value *> allocVar;
        for (auto bb : _currentFunction->getBasicBlocks()) {
            std::set<Value *> alives;
            for (auto succ : bb->getSuccessors()) {
                alives.insert(succ->getAlives().begin(), succ->getAlives().end());
            }
            for (auto rbegin = bb->getInstructionList().rbegin(); rbegin != bb->getInstructionList().rend(); rbegin++) {
                auto inst = *rbegin;
                inst->setIsDead(false);
                switch (inst->getClassId()) {
                    case ID_ALLOC_INST: {
                        auto allocInst = (AllocInst *)inst;
                        if (alives.count(inst->getResult()) == 0) {
                            inst->setIsDead(true);
                        }
                        break;
                    }
                    case ID_STORE_INST: {
                        auto storeInst = (StoreInst *)inst;
                        // store to global var and addr of GEP always alive(it hard to optmize)
                        if (storeInst->getDest()->isGlobal()) {
                            // needn't insert global addr to alives
                            alives.insert(storeInst->getValue());
                        } else if (storeInst->getDest()->getDefined()->getClassId() == ID_GET_ELEMENT_PTR_INST) {
                            alives.insert(storeInst->getValue());
                            alives.insert(storeInst->getDest());
                        } else if (alives.count(storeInst->getDest()) == 0) {
                            inst->setIsDead(true);
                        } else {
                            alives.insert(storeInst->getValue());
                            // elimination to reduce the scale of propagation
                            alives.erase(storeInst->getDest());
                            allocVar.insert(storeInst->getDest());
                        }
                        break;
                    }
                    case ID_FUNCTION_CALL_INST: {
                        // function call always alive(it hard to optmize)
                        auto funCallInst = (FunctionCallInst *)inst;
                        for (auto param : funCallInst->getParams()) {
                            alives.insert(param);
                        }
                        break;
                    }
                    case ID_GET_ELEMENT_PTR_INST: {
                        auto gepInst = (GetElementPtrInst *)inst;
                        if (alives.count(gepInst->getResult()) == 0) {
                            inst->setIsDead(true);
                        } else {
                            alives.insert(gepInst->getPtr());
                            for (auto idx : gepInst->getIndexes()) {
                                alives.insert(idx);
                            }
                        }
                        break;
                    }
                    case ID_BITCAST_INST: {
                        auto bitCastInst = (BitCastInst *)inst;
                        if (alives.count(bitCastInst->getResult()) == 0) {
                            inst->setIsDead(true);
                        } else {
                            if (!bitCastInst->getPtr()->isGlobal()) {
                                alives.insert(bitCastInst->getPtr());
                            }
                        }
                        break;
                    }
                    case ID_RETURN_INST: {
                        alives.insert(static_cast<ReturnInst *>(inst)->getRetValue());
                        break;
                    }
                    case ID_UNARY_INST: {
                        auto unaryInst = (UnaryInst *)inst;
                        if (alives.count(unaryInst->getResult()) == 0) {
                            inst->setIsDead(true);
                        } else {
                            if (!unaryInst->getOperand()->isGlobal()) {
                                alives.insert(unaryInst->getOperand());
                            }
                        }
                        break;
                    }
                    case ID_BINARY_INST: {
                        auto binaryInst = (BinaryInst *)inst;
                        if (alives.count(binaryInst->getResult()) == 0) {
                            inst->setIsDead(true);
                        } else {
                            alives.insert(binaryInst->getOperand1());
                            alives.insert(binaryInst->getOperand2());
                        }
                        break;
                    }
                    case ID_JUMP_INST: {
                        break;
                    }
                    case ID_COND_JUMP_INST: {
                        auto condJumpInst = (CondJumpInst *)inst;
                        alives.insert(condJumpInst->getOperand1());
                        alives.insert(condJumpInst->getOperand2());
                        break;
                    }
                    default:
                        assert(0 && "can't not reach here");
                        break;
                }
                if (inst->getResult()) {
                    alives.erase(inst->getResult());
                }
            }
            if (bb->getAlives() != alives) {
                update = true;
                bb->setAlives(alives);
            }
        }
        auto rbegin = _currentFunction->getBasicBlocks()[0]->getInstructionList().rbegin();
        auto end = _currentFunction->getBasicBlocks()[0]->getInstructionList().rend();
        // restore elimination by store
        for (; rbegin != end; rbegin++) {
            auto inst = *rbegin;
            if (inst->isDead()) {
                switch (inst->getClassId()) {
                    case ID_ALLOC_INST: {
                        auto allocInst = (AllocInst *)inst;
                        if (allocVar.count(inst->getResult()) > 0) {
                            inst->setIsDead(false);
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    } while (update);
}

void IRBuilder::dumpIR(const std::string &filePath) { _currentModule->print(filePath); }

}  // namespace IR
}  // namespace ATC