#include <assert.h>

#include "atomIR/Function.h"

namespace ATC {

namespace AtomIR {

bool AllocInst::AllocForParam = false;
int AllocInst::AllocatedIntParamNum = 0;
int AllocInst::AllocatedFloatParamNum = 0;

AllocInst::AllocInst(Type* allocType, const std::string& resultName) : _allocForParam(AllocForParam) {
    if (AllocForParam) {
        if (allocType->isPointerType() || allocType->getTypeEnum() == INT32_TY) {
            _allocatedIntParamNum = ++AllocatedIntParamNum;
            _allocatedFloatParamNum = AllocatedFloatParamNum;
        } else {
            _allocatedIntParamNum = AllocatedIntParamNum;
            _allocatedFloatParamNum = ++AllocatedFloatParamNum;
        }
    }
    PointerType* ptr = PointerType::get(allocType);
    _result = new Value(ptr, resultName);
    _result->setDefined(this);
}

FunctionCallInst::FunctionCallInst(const FunctionType& functionType, const std::string& funcName,
                                   const std::vector<Value*>& params, const std::string& resultName) {
    assert(functionType._params.size() == params.size());
    for (int i = 0; i < params.size(); i++) {
        assert(functionType._params[i] == params[i]->getType());
    }
    if (functionType._ret->getTypeEnum() == VOID_TY) {
        _result = nullptr;
    } else {
        _result = new Value(functionType._ret, resultName);
        _result->setDefined(this);
    }
    _funcName = funcName;
    _params = params;
}

GetElementPtrInst::GetElementPtrInst(Value* ptr, const std::vector<Value*>& indexes, const std::string& resultName)
    : _ptr(ptr), _indexes(indexes) {
    assert(ptr->getType()->isPointerType() && "should be pointer value");
    if (indexes.size() == 1) {
        _result = new Value(ptr->getType(), resultName);
    } else {
        PointerType* ptrType = (PointerType*)ptr->getType();
        assert(ptrType->getBaseType()->isArrayType() && "shoule be array type");
        _result = new Value(static_cast<ArrayType*>(ptrType->getBaseType())->getBaseType()->getPointerTy(), resultName);
    }
    _result->setDefined(this);
}

BitCastInst::BitCastInst(Value* ptr, Type* destTy) : _ptr(ptr) {
    assert(ptr->getType()->isPointerType() && destTy->isPointerType() && "only pointer can cast to pointer");
    _result = new Value(destTy, "");
    _result->setDefined(this);
}

UnaryInst::UnaryInst(int type, Value* operand, const std::string& resultName) : _type(type), _operand(operand) {
    switch (type) {
        case INST_LOAD:
            assert(_operand->getType()->isPointerType() && "should load from a pointer");
            _result = new Value(static_cast<PointerType*>(_operand->getType())->getBaseType(), resultName);
            break;
        case INST_ITOF:
            _result = new Value(Type::getFloatTy(), resultName);
            break;
        case INST_FTOI:
            _result = new Value(Type::getInt32Ty(), resultName);
            break;
        default:
            assert(false && " should not reach here");
            break;
    }
    _result->setDefined(this);
}

BinaryInst::BinaryInst(int type, Value* operand1, Value* operand2, const std::string& resultName)
    : _type(type), _operand1(operand1), _operand2(operand2) {
    assert(_operand1->getType() == _operand2->getType());
    _intInst = operand1->getType()->getTypeEnum() == INT32_TY;
    switch (type) {
        case INST_ADD:
        case INST_SUB:
        case INST_MUL:
        case INST_DIV:
        case INST_MOD:
        case INST_BIT_AND:
        case INST_BIT_OR:
            _result = new Value(_operand1->getType(), resultName);
            break;
        case INST_LT:
        case INST_LE:
        case INST_GT:
        case INST_GE:
        case INST_EQ:
        case INST_NE:
            _result = new Value(Type::getInt32Ty(), resultName);
            break;
        default:
            assert(false && " should not reach here");
            break;
    }
    _result->setDefined(this);
}

CondJumpInst::CondJumpInst(int type, BasicBlock* trueBB, BasicBlock* falseBB, Value* operand1, Value* operand2)
    : _type(type), _trueBB(trueBB), _falseBB(falseBB), _operand1(operand1), _operand2(operand2) {
    assert(operand1->getType() == operand2->getType());
    _intInst = operand1->getType()->getTypeEnum() == INT32_TY;
}

std::string AllocInst::toString() {
    std::string str;
    str.append(_result->getValueStr())
        .append(" = ")
        .append("alloc")
        .append(" ")
        .append(static_cast<PointerType*>(_result->getType())->getBaseType()->toString());
    return str;
}

std::string StoreInst::toString() {
    std::string str = "store";
    str.append(" ").append(_value->toString()).append(", ").append(_dest->toString());
    return str;
}

std::string FunctionCallInst::toString() {
    std::string str;
    if (_result == nullptr) {
        str.append("call void");
    } else {
        str.append(_result->getValueStr())
            .append(" = ")
            .append("call")
            .append(" ")
            .append(_result->getType()->toString());
    }
    str.append(" @").append(_funcName).append("(");
    for (auto param : _params) {
        str.append(param->toString()).append(", ");
    }
    if (str.back() == ' ') {
        str.pop_back();
        str.pop_back();
    }
    str.append(")");
    return str;
}

std::string GetElementPtrInst::toString() {
    std::string str;
    str.append(_result->getValueStr()).append(" = getelementptr ").append(_ptr->toString());
    for (auto index : _indexes) {
        str.append(", ").append(index->toString());
    }
    return str;
}

std::string BitCastInst::toString() {
    std::string str;
    str.append(_result->getValueStr())
        .append(" = bitcast ")
        .append(_ptr->toString())
        .append(" to ")
        .append(_result->getType()->toString());
    return str;
}

std::string ReturnInst::toString() {
    std::string str = "ret";
    if (_retValue) {
        str.append(" ").append(_retValue->toString());
    }
    return str;
}

std::string UnaryInst::toString() {
    std::string str;
    str.append(_result->getValueStr()).append(" = ");

    switch (_type) {
        case INST_LOAD: {
            PointerType* operandType = (PointerType*)_operand->getType();
            str.append("load")
                .append(" ")
                .append(operandType->getBaseType()->toString())
                .append(", ")
                .append(_operand->toString());
            break;
        }
        case INST_ITOF:
            str.append("itof")
                .append(" ")
                .append(_operand->toString())
                .append(" to ")
                .append(_result->getType()->toString());
            break;
        case INST_FTOI:
            str.append("ftoi")
                .append(" ")
                .append(_operand->toString())
                .append(" to ")
                .append(_result->getType()->toString());
            break;
        default:
            assert(false && " should not reach here");
            break;
    }

    return str;
}

std::string BinaryInst::toString() {
    std::string str;
    str.append(_result->getValueStr()).append(" = ");
    switch (_type) {
        case INST_ADD:
            str.append("add ");
            break;
        case INST_SUB:
            str.append("sub ");
            break;
        case INST_MUL:
            str.append("mul ");
            break;
        case INST_DIV:
            str.append("div ");
            break;
        case INST_MOD:
            str.append("mod ");
            break;
        case INST_BIT_AND:
        case INST_BIT_OR:
            break;
        case INST_LT:
            str.append("lt ");
            break;
        case INST_LE:
            str.append("le ");
            break;
        case INST_GT:
            str.append("gt ");
            break;
        case INST_GE:
            str.append("ge ");
            break;
        case INST_EQ:
            str.append("eq ");
            break;
        case INST_NE:
            str.append("ne ");
            break;
        default:
            assert(false && " should not reach here");
            break;
    }
    str.append(_operand1->toString()).append(", ").append(_operand2->getValueStr());
    return str;
}

std::string JumpInst::toString() {
    std::string str = "jump";
    str.append(" ").append(_targetBB->getBBStr());
    return str;
}

std::string CondJumpInst::toString() {
    std::string str;
    str.append("if").append(" ").append(_operand1->getValueStr());
    switch (_type) {
        case INST_JLT:
            str.append(" < ");
            break;
        case INST_JLE:
            str.append(" <= ");
            break;
        case INST_JGT:
            str.append(" > ");
            break;
        case INST_JGE:
            str.append(" >= ");
            break;
        case INST_JEQ:
            str.append(" == ");
            break;
        case INST_JNE:
            str.append(" != ");
            break;
        default:
            assert(false && " should not reach here");
            break;
    }

    str.append(_operand2->getValueStr())
        .append(" ")
        .append("jump")
        .append(" ")
        .append(_trueBB->getBBStr())
        .append(" ")
        .append("else")
        .append(" ")
        .append("jump")
        .append(" ")
        .append(_falseBB->getBBStr());

    return str;
}

}  // namespace AtomIR
}  // namespace ATC