#include "llvmIR/IRBuilder.h"

#include "AST/CompUnit.h"
#include "AST/Decl.h"
#include "AST/Expression.h"
#include "AST/FunctionDef.h"
#include "AST/Scope.h"
#include "AST/Statement.h"
#include "AST/Variable.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Support/Host.h"

namespace ATC {

IRBuilder::IRBuilder() {
    llvm::LLVMContext *ctx = new llvm::LLVMContext();
    _theIRBuilder = new llvm::IRBuilder<>(*ctx);
    _module = new llvm::Module("module", *ctx);
    _voidTy = llvm::Type::getVoidTy(*ctx);
    _int1Ty = llvm::Type::getInt1Ty(*ctx);
    _int32Ty = llvm::Type::getInt32Ty(*ctx);
    _floatTy = llvm::Type::getFloatTy(*ctx);
    _int32PtrTy = llvm::Type::getInt32PtrTy(*ctx);
    _floatPtrTy = llvm::Type::getFloatPtrTy(*ctx);
    _int1Zero == llvm::ConstantInt::get(_int1Ty, 0);
    _int32Zero = llvm::ConstantInt::get(_int32Ty, 0, true);
    _floatZero = llvm::ConstantFP::get(_floatTy, 0);
    _int1One == llvm::ConstantInt::get(_int1Ty, 1);
    _int32One = llvm::ConstantInt::get(_int32Ty, 1, true);
    _floatOne = llvm::ConstantFP::get(_floatTy, 1);

    _module->setTargetTriple(llvm::sys::getProcessTriple());

    // int getint
    llvm::FunctionType *getint = llvm::FunctionType::get(_int32Ty, {}, false);
    _definedElseWhere["getint"] = llvm::Function::Create(getint, llvm::GlobalValue::ExternalLinkage, "getint", _module);
    // int getch
    llvm::FunctionType *getch = llvm::FunctionType::get(_int32Ty, {}, false);
    _definedElseWhere["getch"] = llvm::Function::Create(getch, llvm::GlobalValue::ExternalLinkage, "getch", _module);
    // float getfloat
    llvm::FunctionType *getfloat = llvm::FunctionType::get(_floatTy, {}, false);
    _definedElseWhere["getfloat"] =
        llvm::Function::Create(getfloat, llvm::GlobalValue::ExternalLinkage, "getfloat", _module);
    // int getarray
    llvm::FunctionType *getarray = llvm::FunctionType::get(_int32Ty, {_int32PtrTy}, false);
    _definedElseWhere["getarray"] =
        llvm::Function::Create(getarray, llvm::GlobalValue::ExternalLinkage, "getarray", _module);
    // int getfarray
    llvm::FunctionType *getfarray = llvm::FunctionType::get(_int32Ty, {_floatPtrTy}, false);
    _definedElseWhere["getfarray"] =
        llvm::Function::Create(getfarray, llvm::GlobalValue::ExternalLinkage, "getfarray", _module);
    // int putint
    llvm::FunctionType *putint = llvm::FunctionType::get(_voidTy, {_int32Ty}, false);
    _definedElseWhere["putint"] = llvm::Function::Create(putint, llvm::GlobalValue::ExternalLinkage, "putint", _module);
    // int putch
    llvm::FunctionType *putch = llvm::FunctionType::get(_voidTy, {_int32Ty}, false);
    _definedElseWhere["putch"] = llvm::Function::Create(putch, llvm::GlobalValue::ExternalLinkage, "putch", _module);
    // int putarray
    llvm::FunctionType *putarray = llvm::FunctionType::get(_voidTy, {_int32Ty, _int32PtrTy}, false);
    _definedElseWhere["putarray"] =
        llvm::Function::Create(putarray, llvm::GlobalValue::ExternalLinkage, "putarray", _module);
    // int putfloat
    llvm::FunctionType *putfloat = llvm::FunctionType::get(_voidTy, {_floatTy}, false);
    _definedElseWhere["putfloat"] =
        llvm::Function::Create(putfloat, llvm::GlobalValue::ExternalLinkage, "putfloat", _module);
    // int putfarray
    llvm::FunctionType *putfarray = llvm::FunctionType::get(_voidTy, {_int32Ty, _floatPtrTy}, false);
    _definedElseWhere["putfarray"] =
        llvm::Function::Create(putfarray, llvm::GlobalValue::ExternalLinkage, "putfarray", _module);
    // int putf
    llvm::FunctionType *putf =
        llvm::FunctionType::get(_voidTy, {llvm::Type::getInt8PtrTy(_module->getContext())}, true);
    _definedElseWhere["putf"] = llvm::Function::Create(putf, llvm::GlobalValue::ExternalLinkage, "putf", _module);
    // int before_main
    llvm::FunctionType *before_main = llvm::FunctionType::get(_voidTy, {}, false);
    _definedElseWhere["before_main"] =
        llvm::Function::Create(before_main, llvm::GlobalValue::ExternalLinkage, "before_main", _module);
    // int after_main
    llvm::FunctionType *after_main = llvm::FunctionType::get(_voidTy, {}, false);
    _definedElseWhere["after_main"] =
        llvm::Function::Create(after_main, llvm::GlobalValue::ExternalLinkage, "after_main", _module);
    // int _sysy_starttime
    llvm::FunctionType *_sysy_starttime = llvm::FunctionType::get(_voidTy, {_int32Ty}, false);
    _definedElseWhere["_sysy_starttime"] =
        llvm::Function::Create(_sysy_starttime, llvm::GlobalValue::ExternalLinkage, "_sysy_starttime", _module);
    // int _sysy_stoptime
    llvm::FunctionType *_sysy_stoptime = llvm::FunctionType::get(_voidTy, {_int32Ty}, false);
    _definedElseWhere["_sysy_stoptime"] =
        llvm::Function::Create(_sysy_stoptime, llvm::GlobalValue::ExternalLinkage, "_sysy_stoptime", _module);
}

IRBuilder::~IRBuilder() {
    //_module->print(llvm::outs(), nullptr);
}

void IRBuilder::visit(FunctionDef *node) {
    std::vector<llvm::Type *> params;
    for (auto param : node->getParams()) {
        // every decl of param expression has only one variable
        DataType *dataType = param->getVariables()[0]->getDataType();
        params.push_back(convertToLLVMType(dataType));
    }
    llvm::FunctionType *funcTy = llvm::FunctionType::get(convertToLLVMType(node->getRetType()), params, false);
    llvm::Function *func = llvm::Function::Create(funcTy, llvm::GlobalValue::ExternalLinkage, node->getName(), _module);
    node->setFunction(func);
    _currentFunction = func;

    llvm::BasicBlock *allocBB = llvm::BasicBlock::Create(_module->getContext(), "init");
    llvm::BasicBlock *entryBB = llvm::BasicBlock::Create(_module->getContext(), "entry");
    allocBB->insertInto(_currentFunction);
    entryBB->insertInto(_currentFunction);

    _theIRBuilder->SetInsertPoint(allocBB);
    allocForScopeVars(node->getScope());
    int i = 0;
    for (auto param : node->getParams()) {
        // the decl of formal param is the only one
        Variable *var = param->getVariables()[0];
        auto arg = func->getArg(i++);
        _theIRBuilder->CreateStore(arg, var->getAddr());
    }
    checkAndCreateBr(entryBB);

    _theIRBuilder->SetInsertPoint(entryBB);
    node->getBlock()->accept(this);

    // if the function didn't execute return before, than return the default value
    if (_hasBrOrRetBlk.find(_theIRBuilder->GetInsertBlock()) == _hasBrOrRetBlk.end()) {
        if (node->getRetType()->getBasicType() == BasicType::INT) {
            _theIRBuilder->CreateRet(_int32Zero);
        } else if (node->getRetType()->getBasicType() == BasicType::FLOAT) {
            _theIRBuilder->CreateRet(_floatZero);
        } else {
            _theIRBuilder->CreateRetVoid();
        }
    }
}

void IRBuilder::visit(Variable *node) {
    if (node->isGlobal()) {
        // create global variable
        llvm::Type *type = convertToLLVMType(node->getDataType());
        _module->getOrInsertGlobal(node->getName(), type);
        auto globalVar = _module->getNamedGlobal(node->getName());

        if (auto initValue = node->getInitValue()) {
            assert(initValue->isConst());
            initValue->accept(this);
            if (initValue->getClassId() == ID_NESTED_EXPRESSION) {
                _value = convertNestedValuesToConstant(static_cast<ArrayType *>(node->getDataType())->getDimensions(),
                                                       0, 0, convertToLLVMType(node->getBasicType()));
                _nestedExpressionValues.clear();
            }
        } else {
            if (node->getDataType()->getClassId() == ID_ARRAY_TYPE) {
                _value = convertNestedValuesToConstant(static_cast<ArrayType *>(node->getDataType())->getDimensions(),
                                                       0, 0, convertToLLVMType(node->getBasicType()));
            } else {
                _value = node->getBasicType() == BasicType::INT ? _int32Zero : _floatZero;
            }
        }
        globalVar->setInitializer((llvm::Constant *)_value);
        node->setAddr(globalVar);
    } else {
        if (auto initValue = node->getInitValue()) {
            initValue->accept(this);
            if (initValue->getClassId() == ID_NESTED_EXPRESSION) {
                auto dimension = static_cast<ArrayType *>(node->getDataType())->getDimensions();
                auto elementSize = static_cast<ArrayType *>(node->getDataType())->getElementSize();
                for (auto item : _nestedExpressionValues) {
                    // get address of the array element from the index
                    int index = item.first;
                    auto addr = node->getAddr();
                    for (int i = 0; i < elementSize.size(); i++) {
                        addr = _theIRBuilder->CreateInBoundsGEP(
                            addr->getType()->getPointerElementType(), addr,
                            {_int32Zero, llvm::ConstantInt::get(_int32Ty, index / elementSize[i])});
                        index -= index / elementSize[i] * elementSize[i];
                    }
                    _theIRBuilder->CreateStore(item.second, addr);
                }
                _nestedExpressionValues.clear();
            } else {
                _value = convertToDestTy(_value, node->getAddr()->getType()->getPointerElementType());
                _theIRBuilder->CreateStore(_value, node->getAddr());
            }
        }
    }
}

void IRBuilder::visit(ConstVal *node) {
    if (node->getBasicType() == BasicType::INT) {
        _value = llvm::ConstantInt::get(_int32Ty, node->getIntValue());
    } else {
        _value = llvm::ConstantFP::get(_floatTy, node->getFloatValue());
    }
}

void IRBuilder::visit(VarRef *node) {
    if (node->isConst()) {
        if (node->getVariable()->getBasicType() == BasicType::INT) {
            _value = llvm::ConstantInt::get(_int32Ty, ExpressionHandle::evaluateConstExpr(node));
        } else {
            _value = llvm::ConstantFP::get(_floatTy, ExpressionHandle::evaluateConstExpr(node));
        }
        return;
    }
    auto addr = node->getVariable()->getAddr();
    if (addr->getType()->getPointerElementType()->isArrayTy()) {
        // cast the array to pointer,
        // int a[10]; 'a' is treated as a pointer when used as a function argument
        _value =
            _theIRBuilder->CreateInBoundsGEP(addr->getType()->getPointerElementType(), addr, {_int32Zero, _int32Zero});
    } else {
        _value = _theIRBuilder->CreateLoad(addr->getType()->getPointerElementType(), addr);
    }
}

void IRBuilder::visit(IndexedRef *node) {
    auto addr = getIndexedRefAddress(node);
    if (addr->getType()->getPointerElementType()->isArrayTy()) {
        // cast the array to pointer
        _value = addr;
    } else {
        _value = _theIRBuilder->CreateLoad(addr->getType()->getPointerElementType(), addr);
    }
}

void IRBuilder::visit(NestedExpression *node) {
    static int deep = 0;
    static int index;
    static bool scalar;
    // dimensions of definded variable
    static std::vector<int> dimensions;

    if (deep == 0) {
        index = 0;
        scalar = false;
        dimensions.clear();
        _nestedExpressionValues.clear();
        assert(node->getParent()->getClassId() == ID_VARIABLE);
        auto var = (Variable *)node->getParent();
        assert(var->getDataType()->getClassId() == ID_ARRAY_TYPE);
        dimensions = ((ArrayType *)var->getDataType())->getDimensions();
    }

    // The maximum number of elements in a nested expression
    int maxSize = 1;
    for (int i = deep; i < dimensions.size(); i++) {
        maxSize *= dimensions[i];
    }
    int targetIndex = index + maxSize;
    deep++;
    const auto &elements = node->getElements();
    // get one value when there are excess elements in a scalar initializer or
    // when there are too many braces around a scalar initializer.
    if (scalar || deep > dimensions.size()) {
        if (elements[0]->getClassId() == ID_NESTED_EXPRESSION) {
            bool tmp = scalar;
            elements[0]->accept(this);
            scalar = tmp;
        } else {
            elements[0]->accept(this);
            _nestedExpressionValues.insert({index++, _value});
        }
    } else {
        for (size_t i = 0; i < elements.size(); i++) {
            if (elements[i]->getClassId() == ID_NESTED_EXPRESSION) {
                bool tmp = scalar;
                elements[i]->accept(this);
                scalar = tmp;
            } else {
                scalar = true;
                elements[i]->accept(this);
                _nestedExpressionValues.insert({index++, _value});
                // ignore the remaining elements
                if (index == targetIndex) {
                    break;
                }
            }
        }
        index = targetIndex;
    }
    deep--;
}

void IRBuilder::visit(UnaryExpression *node) {
    node->getOperand()->accept(this);
    if (node->getOperator() == MINUS) {
        if (_value->getType() == _floatTy) {
            _value = _theIRBuilder->CreateFSub(_floatZero, _value);
        } else if (_value->getType() == _int32Ty) {
            _value = _theIRBuilder->CreateSub(_int32Zero, _value);
        }
    } else if (node->getOperator() == NOT) {
        _value = _theIRBuilder->CreateNot(convertToDestTy(_value, _int1Ty));
    }
}

void IRBuilder::visit(BinaryExpression *node) {
    if (node->isShortCircuit()) {
        llvm::BasicBlock *rhsCondBB = llvm::BasicBlock::Create(_module->getContext(), "rhsCondBB");
        rhsCondBB->insertInto(_currentFunction);

        if (node->getOperator() == AND) {
            auto tmpTrueBB = _trueBB;
            _trueBB = rhsCondBB;
            node->getLeft()->accept(this);
            checkAndCreateCondBr(convertToDestTy(_value, _int1Ty), rhsCondBB, _falseBB);
            _trueBB = tmpTrueBB;
        } else {
            auto tmpFalseBB = _falseBB;
            _falseBB = rhsCondBB;
            node->getLeft()->accept(this);
            checkAndCreateCondBr(convertToDestTy(_value, _int1Ty), _trueBB, rhsCondBB);
            _falseBB = tmpFalseBB;
        }

        _theIRBuilder->SetInsertPoint(rhsCondBB);
        node->getRight()->accept(this);
        return;
    }

    node->getLeft()->accept(this);
    auto left = _value;
    node->getRight()->accept(this);
    auto right = _value;

    if (node->getOperator() == AND) {
        _value = _theIRBuilder->CreateAnd(convertToDestTy(left, _int1Ty), convertToDestTy(right, _int1Ty));
        return;
    } else if (node->getOperator() == OR) {
        _value = _theIRBuilder->CreateOr(convertToDestTy(left, _int1Ty), convertToDestTy(right, _int1Ty));
        return;
    }

    // make the left expr and right expr has the same type
    if (left->getType() == _floatTy || right->getType() == _floatTy) {
        left = convertToDestTy(left, _floatTy);
        right = convertToDestTy(right, _floatTy);
        switch (node->getOperator()) {
            case PLUS:
                _value = _theIRBuilder->CreateFAdd(left, right);
                break;
            case MINUS:
                _value = _theIRBuilder->CreateFSub(left, right);
                break;
            case MUL:
                _value = _theIRBuilder->CreateFMul(left, right);
                break;
            case DIV:
                _value = _theIRBuilder->CreateFDiv(left, right);
                break;
            case MOD:
                assert(false && "should not reach here");
                break;
            case LT:
                _value = _theIRBuilder->CreateFCmpOLT(left, right);
                break;
            case GT:
                _value = _theIRBuilder->CreateFCmpOGT(left, right);
                break;
            case LE:
                _value = _theIRBuilder->CreateFCmpOLE(left, right);
                break;
            case GE:
                _value = _theIRBuilder->CreateFCmpOGE(left, right);
                break;
            case EQ:
                _value = _theIRBuilder->CreateFCmpOEQ(left, right);
                break;
            case NE:
                _value = _theIRBuilder->CreateFCmpONE(left, right);
                break;
            default:
                assert(false && "should not reach here");
                break;
        }
    } else {
        left = convertToDestTy(left, _int32Ty);
        right = convertToDestTy(right, _int32Ty);
        switch (node->getOperator()) {
            case PLUS:
                _value = _theIRBuilder->CreateAdd(left, right);
                break;
            case MINUS:
                _value = _theIRBuilder->CreateSub(left, right);
                break;
            case MUL:
                _value = _theIRBuilder->CreateMul(left, right);
                break;
            case DIV:
                _value = _theIRBuilder->CreateSDiv(left, right);
                break;
            case MOD:
                _value = _theIRBuilder->CreateSRem(left, right);
                break;
            case LT:
                _value = _theIRBuilder->CreateICmpSLT(left, right);
                break;
            case GT:
                _value = _theIRBuilder->CreateICmpSGT(left, right);
                break;
            case LE:
                _value = _theIRBuilder->CreateICmpSLE(left, right);
                break;
            case GE:
                _value = _theIRBuilder->CreateICmpSGE(left, right);
                break;
            case EQ:
                _value = _theIRBuilder->CreateICmpEQ(left, right);
                break;
            case NE:
                _value = _theIRBuilder->CreateICmpNE(left, right);
                break;
            default:
                assert(false && "should not reach here");
                break;
        }
    }
}

void IRBuilder::visit(FunctionCall *node) {
    std::vector<llvm::Value *> params;
    llvm::Function *function = nullptr;
    if (node->getFunctionDef()) {
        function = node->getFunctionDef()->getFunction();
    } else {
        function = _definedElseWhere[node->getName()];
    }
    int i = 0;
    for (auto rParam : node->getParams()) {
        rParam->accept(this);
        _value = _theIRBuilder->CreateBitCast(_value, function->getArg(i++)->getType());
        params.push_back(_value);
    }
    _value = _theIRBuilder->CreateCall(function, params);
}

void IRBuilder::visit(Block *node) {
    for (auto element : node->getElements()) {
        element->accept(this);
        // skip the statements following the return statement
        if (element->getClassId() == ID_RETURN_STATEMENT || element->getClassId() == ID_CONTINUE_STATEMENT ||
            element->getClassId() == ID_BREAK_STATEMENT) {
            break;
        }
    }
}

void IRBuilder::visit(AssignStatement *node) {
    llvm::Value *addr = nullptr;
    if (node->getLval()->getClassId() == ID_VAR_REF) {
        addr = static_cast<VarRef *>(node->getLval())->getVariable()->getAddr();
    } else {
        addr = getIndexedRefAddress((IndexedRef *)node->getLval());
    }
    node->getRval()->accept(this);
    _value = convertToDestTy(_value, addr->getType()->getPointerElementType());
    _theIRBuilder->CreateStore(_value, addr);
}

void IRBuilder::visit(IfStatement *node) {
    llvm::BasicBlock *ifBB = llvm::BasicBlock::Create(_module->getContext(), "ifBB");
    llvm::BasicBlock *afterIfBB = llvm::BasicBlock::Create(_module->getContext(), "afterIfBB");
    ifBB->insertInto(_currentFunction);
    afterIfBB->insertInto(_currentFunction);

    _trueBB = ifBB;
    if (node->getElseStmt()) {
        llvm::BasicBlock *elseBB = llvm::BasicBlock::Create(_module->getContext(), "elseBB");
        elseBB->insertInto(_currentFunction);
        _falseBB = elseBB;
        node->getCond()->accept(this);
        checkAndCreateCondBr(convertToDestTy(_value, _int1Ty), _trueBB, _falseBB);

        _theIRBuilder->SetInsertPoint(elseBB);
        node->getElseStmt()->accept(this);
        checkAndCreateBr(afterIfBB);
    } else {
        _falseBB = afterIfBB;
        node->getCond()->accept(this);
        checkAndCreateCondBr(convertToDestTy(_value, _int1Ty), _trueBB, _falseBB);
    }

    _theIRBuilder->SetInsertPoint(ifBB);
    node->getStmt()->accept(this);
    checkAndCreateBr(afterIfBB);

    _theIRBuilder->SetInsertPoint(afterIfBB);
}

void IRBuilder::visit(WhileStatement *node) {
    llvm::BasicBlock *condBB = llvm::BasicBlock::Create(_module->getContext(), "condBB");
    llvm::BasicBlock *whileBB = llvm::BasicBlock::Create(_module->getContext(), "whileBB");
    llvm::BasicBlock *afterWhileBB = llvm::BasicBlock::Create(_module->getContext(), "afterWhileBB");
    condBB->insertInto(_currentFunction);
    whileBB->insertInto(_currentFunction);
    afterWhileBB->insertInto(_currentFunction);

    checkAndCreateBr(condBB);
    _theIRBuilder->SetInsertPoint(condBB);
    _trueBB = whileBB;
    _falseBB = afterWhileBB;
    node->getCond()->accept(this);
    checkAndCreateCondBr(convertToDestTy(_value, _int1Ty), _trueBB, _falseBB);

    auto tmpCondBB = _condBB;
    auto tmpAfterBB = _afterBB;
    _condBB = condBB;
    _afterBB = afterWhileBB;
    _theIRBuilder->SetInsertPoint(whileBB);
    node->getStmt()->accept(this);
    checkAndCreateBr(condBB);
    _condBB = tmpCondBB;
    _afterBB = tmpAfterBB;

    _theIRBuilder->SetInsertPoint(afterWhileBB);
}

void IRBuilder::visit(BreakStatement *node) { checkAndCreateBr(_afterBB); }

void IRBuilder::visit(ContinueStatement *node) { checkAndCreateBr(_condBB); }

void IRBuilder::visit(ReturnStatement *node) {
    _hasBrOrRetBlk.insert(_theIRBuilder->GetInsertBlock());
    if (node->getExpr()) {
        node->getExpr()->accept(this);
        _theIRBuilder->CreateRet(convertToDestTy(_value, _currentFunction->getReturnType()));
    } else {
        _theIRBuilder->CreateRetVoid();
    }
}

llvm::Type *IRBuilder::convertToLLVMType(int basicType) {
    switch (basicType) {
        case BasicType::BOOL:
            return _int1Ty;
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

llvm::Type *IRBuilder::convertToLLVMType(DataType *dataType) {
    if (dataType->getClassId() == ID_POINTER_TYPE) {
        return llvm::PointerType::get(convertToLLVMType(dataType->getBaseDataType()), 0);
    } else if (dataType->getClassId() == ID_ARRAY_TYPE) {
        auto arrayType = (ArrayType *)dataType;
        const auto &dimensions = arrayType->getDimensions();
        llvm::Type *tmpType = convertToLLVMType(arrayType->getBaseDataType());
        for (auto rbegin = dimensions.rbegin(); rbegin != dimensions.rend(); rbegin++) {
            tmpType = llvm::ArrayType::get(tmpType, *rbegin);
        }
        return tmpType;
    } else {
        return convertToLLVMType(dataType->getBasicType());
    }
}

void IRBuilder::allocForScopeVars(Scope *currentScope) {
    for (const auto &[name, var] : currentScope->getVarMap()) {
        llvm::Value *addr = nullptr;
        addr = _theIRBuilder->CreateAlloca(convertToLLVMType(var->getDataType()), nullptr, name);
        var->setAddr(addr);
    }
    for (auto child : currentScope->getChildren()) {
        allocForScopeVars(child);
    }
}

llvm::Value *IRBuilder::convertToDestTy(llvm::Value *value, llvm::Type *destTy) {
    if (destTy == _floatTy) {
        if (value->getType() == _int32Ty) {
            return _theIRBuilder->CreateSIToFP(value, _floatTy);
        } else if (_value->getType() == _int1Ty) {
            return _theIRBuilder->CreateUIToFP(value, _floatTy);
        }
    } else if (destTy == _int32Ty) {
        if (value->getType() == _floatTy) {
            return _theIRBuilder->CreateFPToSI(value, _int32Ty);
        } else if (value->getType() == _int1Ty) {
            return _theIRBuilder->CreateZExt(value, _int32Ty);
        }
    } else {
        if (value->getType() == _floatTy) {
            return _theIRBuilder->CreateFCmpONE(value, _floatZero);
        } else if (value->getType() == _int32Ty) {
            return _theIRBuilder->CreateICmpNE(value, _int32Zero);
        }
    }
    return value;
}

void IRBuilder::checkAndCreateBr(llvm::BasicBlock *destBlk) {
    if (_hasBrOrRetBlk.find(_theIRBuilder->GetInsertBlock()) == _hasBrOrRetBlk.end()) {
        _theIRBuilder->CreateBr(destBlk);
        _hasBrOrRetBlk.insert(_theIRBuilder->GetInsertBlock());
    }
}

void IRBuilder::checkAndCreateCondBr(llvm::Value *value, llvm::BasicBlock *trueBlk, llvm::BasicBlock *falseBlck) {
    if (_hasBrOrRetBlk.find(_theIRBuilder->GetInsertBlock()) == _hasBrOrRetBlk.end()) {
        _theIRBuilder->CreateCondBr(value, _trueBB, _falseBB);
        _hasBrOrRetBlk.insert(_theIRBuilder->GetInsertBlock());
    }
}

llvm::Value *IRBuilder::getIndexedRefAddress(IndexedRef *indexedRef) {
    Variable *var = indexedRef->getVariable();
    llvm::Value *addr = var->getAddr();
    assert(addr->getType()->isPointerTy());
    const auto &dimension = indexedRef->getDimensions();
    auto begin = dimension.begin();
    if (var->getDataType()->getClassId() == ID_POINTER_TYPE) {
        addr = _theIRBuilder->CreateLoad(addr->getType()->getPointerElementType(), addr);
        (*begin++)->accept(this);
        addr = _theIRBuilder->CreateInBoundsGEP(addr->getType()->getPointerElementType(), addr, _value);
    }
    for (; begin != dimension.end(); begin++) {
        (*begin)->accept(this);
        addr = _theIRBuilder->CreateInBoundsGEP(addr->getType()->getPointerElementType(), addr, {_int32Zero, _value});
    }
    return addr;
}

llvm::Value *IRBuilder::convertNestedValuesToConstant(const std::vector<int> &dimensions, int deep, int begin,
                                                      llvm::Type *basicType) {
    if (deep == dimensions.size()) {
        return _nestedExpressionValues[begin];
    }

    llvm::Type *partType = basicType;
    for (int i = dimensions.size() - 1; i > deep; i--) {
        partType = llvm::ArrayType::get(partType, dimensions[i]);
    }
    llvm::Constant *zeroInitializer = llvm::ConstantAggregateZero::get(partType);

    std::vector<llvm::Constant *> ret;
    int partSize = 1;
    for (int i = deep + 1; i < dimensions.size(); i++) {
        partSize *= dimensions[i];
    }

    int right = begin + dimensions[deep] * partSize;
    int left = right - partSize;

    // Traverse from the rear to reduce the 'push zeroInitializer' operation
    bool valid = false;
    for (int i = dimensions[deep] - 1; i >= 0; i--) {
        bool pushed = false;
        for (auto item : _nestedExpressionValues) {
            if (item.first >= left && item.first < right) {
                ret.push_back((llvm::Constant *)convertNestedValuesToConstant(dimensions, deep + 1, left, basicType));
                valid = true;
                pushed = true;
                break;
            }
        }
        if (valid && !pushed) {
            ret.push_back(zeroInitializer);
        }
        left -= partSize;
        right -= partSize;
    }

    std::reverse(ret.begin(), ret.end());

    if (ret.size() != dimensions[deep]) {
        // create the type for structType
        std::vector<llvm::Type *> elementTypes(ret.size(), partType);
        // type of remain elements
        llvm::ArrayType *remainType = llvm::ArrayType::get(partType, dimensions[deep] - ret.size());
        elementTypes.push_back(remainType);
        ret.push_back(llvm::ConstantAggregateZero::get(remainType));
        llvm::StructType *retType = llvm::StructType::create(elementTypes);
        return llvm::ConstantStruct::get(retType, ret);
    }
    return llvm::ConstantArray::get(llvm::ArrayType::get(partType, ret.size()), ret);
}
}  // namespace ATC