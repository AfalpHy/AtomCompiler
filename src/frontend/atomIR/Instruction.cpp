#include "atomIR/Instruction.h"

#include "assert.h"
#include "atomIR/Function.h"

namespace ATC {

namespace AtomIR {

AllocInst::AllocInst(Type* allocType, const std::string& resultName) : Instruction(INST_ALLOC) {
    PointerType* ptr = PointerType::get(allocType);
    _result = new Value(ptr, resultName);
    _result->setDefined(this);
}

FunctionCallInst::FunctionCallInst(FunctionType functionType, const std::string& funcName,
                                   const std::vector<Value*>& params, const std::string& resultName)
    : Instruction(INST_FUNCALL) {
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

UnaryInst::UnaryInst(InstType type, Value* operand, const std::string& resultName)
    : Instruction(type), _operand(operand), _result(new Value(resultName)) {
    switch (type) {
        case INST_LOAD:
            assert(_operand->getType()->isPointerType() && "should load from a pointer");
            _result->setType(static_cast<PointerType*>(_operand->getType())->getBaseType());
            break;
        case INST_ITOF:
            _result->setType(Type::getFloatTy());
            break;
        case INST_FTOI:
            _result->setType(Type::getInt32Ty());
            break;
        default:
            assert(false && " should not reach here");
            break;
    }
    _result->setDefined(this);
}

BinaryInst::BinaryInst(InstType type, Value* operand1, Value* operand2, const std::string& resultName)
    : Instruction(type), _operand1(operand1), _operand2(operand2), _result(new Value(resultName)) {
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
            _result->setType(_operand1->getType());
            break;
        case INST_LT:
        case INST_LE:
        case INST_GT:
        case INST_GE:
        case INST_EQ:
        case INST_NE:
            _result->setType(Type::getInt1Ty());
            break;
        default:
            assert(false && " should not reach here");
            break;
    }
    _result->setDefined(this);
}

CondJumpInst::CondJumpInst(InstType type, BasicBlock* trueBB, BasicBlock* falseBB, Value* operand1, Value* operand2)
    : Instruction(type), _trueBB(trueBB), _falseBB(falseBB), _operand1(operand1), _operand2(operand2) {
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
    str.append(" ").append(_funcName).append("(");
    for (auto param : _params) {
        str.append(param->toString()).append(",");
    }
    if (str.back() == ',') {
        str.pop_back();
    }
    str.append(")");
    return str;
}

std::string ReturnInst::toString() {
    std::string str = "ret";
    str.append(" ").append(_retValue->toString());
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
            break;
        case INST_FTOI:
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
        case INST_MUL:
        case INST_DIV:
        case INST_MOD:
        case INST_BIT_AND:
        case INST_BIT_OR:
            break;
        case INST_LT:
        case INST_LE:
        case INST_GT:
        case INST_GE:
        case INST_EQ:
        case INST_NE:
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
            str.append(" LT ");
            break;
        case INST_JLE:
            str.append(" LE ");
            break;
        case INST_JGT:
            str.append(" GT ");
            break;
        case INST_JGE:
            str.append(" GE ");
            break;
        case INST_JEQ:
            str.append(" EQ ");
            break;
        case INST_JNE:
            str.append(" NE ");
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