#include "llvmIR/IRBuilder.h"

#include "AST/CompUnit.h"
#include "AST/Decl.h"
#include "AST/Expression.h"
#include "AST/FunctionDef.h"
#include "AST/Scope.h"
#include "AST/Statement.h"
#include "AST/Variable.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Support/raw_ostream.h"
namespace ATC {

bool isAndOrExpr(Expression *expr) {
    if (expr->getClassId() != ID_BINARY_EXPRESSION) {
        return false;
    } else {
        auto binaryExpr = (BinaryExpression *)expr;
        return binaryExpr->getOperator() == AND || binaryExpr->getOperator() == OR;
    }
}

IRBuilder::IRBuilder() {
    llvm::LLVMContext *ctx = new llvm::LLVMContext();
    _theIRBuilder = new llvm::IRBuilder<>(*ctx);
    _module = new llvm::Module("module", *ctx);
    _voidTy = llvm::Type::getVoidTy(*ctx);
    _int32Ty = llvm::Type::getInt32Ty(*ctx);
    _floatTy = llvm::Type::getFloatTy(*ctx);
    _int32PtrTy = llvm::Type::getInt32PtrTy(*ctx);
    _floatPtrTy = llvm::Type::getFloatPtrTy(*ctx);
    _int32Zero = llvm::ConstantInt::get(_int32Ty, 0, true);
    _floatZero = llvm::ConstantFP::get(_floatTy, 0);
    _int32One = llvm::ConstantInt::get(_int32Ty, 1, true);
    _floatOne = llvm::ConstantFP::get(_floatTy, 1);
}

IRBuilder::~IRBuilder() { _module->print(llvm::outs(), nullptr); }

void IRBuilder::visit(FunctionDef *node) {
    std::vector<llvm::Type *> params;
    for (auto param : node->getParams()) {
        DataType *dataType = param->getDataType();
        params.push_back(convetToLLVMType(dataType));
    }
    llvm::FunctionType *funcTy = llvm::FunctionType::get(convetToLLVMType(node->getRetType()), params, false);
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
    _theIRBuilder->CreateBr(entryBB);

    _theIRBuilder->SetInsertPoint(entryBB);
    node->getBlock()->accept(this);

    // if the function didn't execute return before, than return the default value
    if (node->getRetType() == INT) {
        _theIRBuilder->CreateRet(_int32Zero);
    } else {
        _theIRBuilder->CreateRet(_floatZero);
    }
}

void IRBuilder::visit(Variable *node) {
    if (node->isGlobal()) {
        // create global variable
        llvm::Type *type = node->getDataType()->getBaseType() == INT ? _int32Ty : _floatTy;
        _module->getOrInsertGlobal(node->getName(), type);
        auto globalVar = _module->getNamedGlobal(node->getName());

        if (auto initValue = node->getInitValue()) {
            assert(initValue->isConst());
            initValue->accept(this);
            globalVar->setInitializer((llvm::Constant *)_value);
        }
        node->setAddr(globalVar);
    } else {
        if (auto initValue = node->getInitValue()) {
            initValue->accept(this);
            _theIRBuilder->CreateStore(_value, node->getAddr());
        }
    }
}

void IRBuilder::visit(ConstVal *node) {
    if (node->getBaseType() == INT) {
        _value = llvm::ConstantInt::get(_int32Ty, node->getIntValue());
    } else {
        _value = llvm::ConstantFP::get(_floatTy, node->getFloatValue());
    }
}

void IRBuilder::visit(VarRef *node) { _value = _theIRBuilder->CreateLoad(_int32Ty, node->getVariable()->getAddr()); }

void IRBuilder::visit(ArrayExpression *node) {
    if (node->isConst()) {
        // std::vector<llvm::Constant *> array;
        // array.push_back(_int32One);
        // array.push_back(_int32One);
        // llvm::ArrayRef<llvm::Constant *> arrayRef = array;
        // auto arrayType = llvm::ArrayType::get(_int32Ty, 0);
        // auto arrayType1 = llvm::ArrayType::get(arrayType, 0);
        // auto tmp1 = llvm::ConstantArray::get(arrayType, array);
        // std::vector<llvm::Constant *> array1;
        // array1.push_back(tmp1);
        // array1.push_back(tmp1);
        // _value = llvm::ConstantArray::get(arrayType, array1);
    }
}

void IRBuilder::visit(UnaryExpression *node) {
    node->getOperand()->accept(this);
    switch (node->getOperator()) {
        case PLUS:
            break;
        case MINUS:
            _value = _theIRBuilder->CreateSub(_int32Zero, _value);
            break;
        case NOT:
            _value = _theIRBuilder->CreateNot(_value);
            break;
        default:
            assert(false && "should not reach here");
            break;
    }
}

void IRBuilder::visit(BinaryExpression *node) {
    if (node->getOperator() == AND || node->getOperator() == OR) {
        llvm::BasicBlock *rhsCondBB = llvm::BasicBlock::Create(_module->getContext(), "rhsCondBB");
        rhsCondBB->insertInto(_currentFunction);

        // if lhs is a single cond expression
        if (!isAndOrExpr(node->getLeft())) {
            node->getLeft()->accept(this);
            if (node->getOperator() == AND) {
                _theIRBuilder->CreateCondBr(_value, rhsCondBB, _falseBB);
            } else {
                _theIRBuilder->CreateCondBr(_value, _trueBB, rhsCondBB);
            }
        } else {
            if (node->getOperator() == AND) {
                auto tmpTrueBB = _trueBB;
                _trueBB = rhsCondBB;
                node->getLeft()->accept(this);
                _trueBB = tmpTrueBB;
            } else {
                auto tmpFalseBB = _falseBB;
                _falseBB = rhsCondBB;
                node->getLeft()->accept(this);
                _falseBB = tmpFalseBB;
            }
        }

        _theIRBuilder->SetInsertPoint(rhsCondBB);
        node->getRight()->accept(this);
        if (!isAndOrExpr(node->getRight())) {
            _theIRBuilder->CreateCondBr(_value, _trueBB, _falseBB);
        }
        return;
    }
    if (node->isConst()) {
        auto value = Expression::evaluateConstExpr(node);
        _value = llvm::ConstantInt::get(_int32Ty, value);
        return;
    }

    node->getLeft()->accept(this);
    auto left = _value;
    node->getRight()->accept(this);
    auto right = _value;
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
        case AND:
            _value = _theIRBuilder->CreateAnd(left, right);
            break;
        case OR:
            _value = _theIRBuilder->CreateOr(left, right);
            break;
        default:
            assert(false && "should not reach here");
            break;
    }
}

void IRBuilder::visit(FunctionCall *node) {
    std::vector<llvm::Value *> params;
    for (auto rParam : node->getParams()) {
        rParam->accept(this);
        params.push_back(_value);
    }
    _value = _theIRBuilder->CreateCall(node->getFunctionDef()->getFunction(), params);
}

void IRBuilder::visit(AssignStatement *node) {
    node->getValue()->accept(this);
    _theIRBuilder->CreateStore(_value, node->getVar()->getVariable()->getAddr());
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
        if (!isAndOrExpr(node->getCond())) {
            _theIRBuilder->CreateCondBr(_value, _trueBB, _falseBB);
        }
        _theIRBuilder->SetInsertPoint(elseBB);
        node->getElseStmt()->accept(this);
        _theIRBuilder->CreateBr(afterIfBB);
    } else {
        _falseBB = afterIfBB;
        node->getCond()->accept(this);
        if (!isAndOrExpr(node->getCond())) {
            _theIRBuilder->CreateCondBr(_value, _trueBB, _falseBB);
        }
    }

    _theIRBuilder->SetInsertPoint(ifBB);
    node->getStmt()->accept(this);
    _theIRBuilder->CreateBr(afterIfBB);

    _theIRBuilder->SetInsertPoint(afterIfBB);
}

void IRBuilder::visit(WhileStatement *node) {
    llvm::BasicBlock *condBB = llvm::BasicBlock::Create(_module->getContext(), "condBB");
    llvm::BasicBlock *whileBB = llvm::BasicBlock::Create(_module->getContext(), "whileBB");
    llvm::BasicBlock *afterWhileBB = llvm::BasicBlock::Create(_module->getContext(), "afterWhileBB");
    condBB->insertInto(_currentFunction);
    whileBB->insertInto(_currentFunction);
    afterWhileBB->insertInto(_currentFunction);

    _theIRBuilder->CreateBr(condBB);
    _theIRBuilder->SetInsertPoint(condBB);
    _trueBB = whileBB;
    _falseBB = afterWhileBB;
    node->getCond()->accept(this);
    if (!isAndOrExpr(node->getCond())) {
        _theIRBuilder->CreateCondBr(_value, _trueBB, _falseBB);
    }

    _theIRBuilder->SetInsertPoint(whileBB);
    node->getStmt()->accept(this);
    _theIRBuilder->CreateBr(condBB);

    _theIRBuilder->SetInsertPoint(afterWhileBB);
}

void IRBuilder::visit(ReturnStatement *node) {
    if (node->getExpr()) {
        node->getExpr()->accept(this);
        _theIRBuilder->CreateRet(_value);
    } else {
        _theIRBuilder->CreateRetVoid();
    }
}

llvm::Type *IRBuilder::convetToLLVMType(int type) {
    switch (type) {
        case INT:
            return _int32Ty;
        case FLOAT:
            return _floatTy;
        case VOID:
            return _voidTy;
        default:
            assert(false && "should not reach here");
    }
}

llvm::Type *IRBuilder::convetToLLVMType(DataType *dataType) {
    if (dataType->isPointer()) {
        // TODO:
        return nullptr;
    } else {
        return convetToLLVMType(dataType->getBaseType());
    }
}

void IRBuilder::allocForScopeVars(Scope *currentScope) {
    for (auto [name, var] : currentScope->getVarMap()) {
        auto addr = _theIRBuilder->CreateAlloca(convetToLLVMType(var->getDataType()), nullptr, name);
        var->setAddr(addr);
    }
    for (auto child : currentScope->getChildren()) {
        allocForScopeVars(child);
    }
}

}  // namespace ATC