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
    llvm::FunctionType *funcTy =
        llvm::FunctionType::get(convetToLLVMType(node->getRetType()), params, false);
    llvm::Function *func = llvm::Function::Create(funcTy, llvm::GlobalValue::ExternalLinkage,
                                                  node->getName(), _module);
    node->setFunction(func);

    llvm::BasicBlock *allocBB = llvm::BasicBlock::Create(_module->getContext(), "init");
    llvm::BasicBlock *entryBB = llvm::BasicBlock::Create(_module->getContext(), "entry");
    allocBB->insertInto(func);
    entryBB->insertInto(func);

    _theIRBuilder->SetInsertPoint(allocBB);
    allocForScopeVars(node->getScope());
    int i = 0;
    for (auto param : node->getParams()) {
        // 形参中的每个声明只能有一个定义
        Variable *var = param->getVariables()[0];
        auto arg = func->getArg(i++);
        _theIRBuilder->CreateStore(arg, var->getAddr());
    }
    _theIRBuilder->CreateBr(entryBB);

    _theIRBuilder->SetInsertPoint(entryBB);
    node->getBlock()->accept(this);
    _theIRBuilder->CreateRet(_int32Zero);
}

void IRBuilder::visit(DataType *node) {}

void IRBuilder::visit(Variable *node) {
    llvm::Type *type = node->getDataType()->getBaseType() == INT ? _int32Ty : _floatTy;
    if (node->isGlobal()) {
        // create global variable
        _module->getOrInsertGlobal(node->getName(), type);
        auto globalVar = _module->getNamedGlobal(node->getName());

        if (auto initValue = node->getInitValue()) {
            assert(initValue->isConst());
            auto value = Expression::evaluateConstExpr(initValue);
            globalVar->setInitializer(llvm::ConstantInt::get(type, value));
        }
        node->setAddr(globalVar);
    } else {
        if (auto initValue = node->getInitValue()) {
            if (initValue->isConst()) {
                auto value = Expression::evaluateConstExpr(initValue);
                _theIRBuilder->CreateStore(llvm::ConstantInt::get(type, value), node->getAddr());
            } else {
                initValue->accept(this);
                _theIRBuilder->CreateStore(_value, node->getAddr());
            }
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

void IRBuilder::visit(VarRef *node) {
    _value = _theIRBuilder->CreateLoad(_int32Ty, node->getVariable()->getAddr());
}

void IRBuilder::visit(ArrayExpression *node) {}

void IRBuilder::visit(UnaryExpression *node) {}

void IRBuilder::visit(BinaryExpression *node) {
    node->getLeft()->accept(this);
    auto left = _value;
    node->getRight()->accept(this);
    auto right = _value;
    switch (node->getOperator()) {
        case PLUS:
            _value = _theIRBuilder->CreateAdd(left, right);
            break;

        default:
            break;
    }
}

void IRBuilder::visit(FunctionCall *node) {}

void IRBuilder::visit(AssignStatement *node) {}

void IRBuilder::visit(IfStatement *node) {}

void IRBuilder::visit(ElseStatement *node) {}

void IRBuilder::visit(WhileStatement *node) {}

void IRBuilder::visit(ReturnStatement *node) {}

void IRBuilder::visit(OtherStatement *node) {}

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
        auto addr =
            _theIRBuilder->CreateAlloca(convetToLLVMType(var->getDataType()), nullptr, name);
        var->setAddr(addr);
    }
    for (auto child : currentScope->getChildren()) {
        allocForScopeVars(child);
    }
}
}  // namespace ATC