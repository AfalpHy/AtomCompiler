#include "atomIR/Instruction.h"

#include "assert.h"

namespace ATC {

namespace AtomIR {

AllocInst::AllocInst(Type* allocType, const std::string& resultName) : Instruction(INST_ALLOC) {
    PointerType* ptr = new PointerType(allocType);
    _result = new Value(ptr, resultName);
}

UnaryInst::UnaryInst(InstType type, Value* operand, const std::string& resultName)
    : Instruction(type), _operand(operand), _result(new Value(resultName)) {
    switch (type) {
        case INST_LOAD:
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
            _result->setType(_operand1->getType());
            break;
        case INST_LT:
        case INST_LE:
        case INST_GT:
        case INST_GE:
        case INST_EQ:
        case INST_NE:
        case INST_AND:
        case INST_OR:
            _result->setType(Type::getInt1Ty());
            break;
        default:
            assert(false && " should not reach here");
            break;
    }
}

CondJumpInst::CondJumpInst(InstType type, BasicBlock* trueBB, BasicBlock* falseBB, Value* operand1, Value* operand2)
    : Instruction(type), _trueBB(trueBB), _falseBB(falseBB), _operand1(operand1), _operand2(operand2) {
    assert(operand1->getType() == operand2->getType());
    _intInst = operand1->getType()->getTypeEnum() == INT32_TY;
}

std::string AllocInst::toString() {
    std::string str;
    str.append(_result->getName()).append(" = ").append("alloc").append(" ").append("int32");
    return str;
}

std::string StoreInst::toString() {
    std::string str = "store";
    str.append(" ").append(_value->getName()).append(" to ").append(_dest->getName());
    return str;
}

std::string ReturnInst::toString() {
    std::string str = "ret";
    str.append(" ").append(_retValue->getName());
    return str;
}

std::string UnaryInst::toString() {
    std::string str;
    str.append(_result->getName()).append(" = ");

    switch (_type) {
        case INST_LOAD:
            str.append("load").append(" ").append(_operand->getName());
            break;
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
    str.append(_result->getName()).append(" = ");
    switch (_type) {
        case INST_ADD:
            str.append("add ");
            break;
        case INST_SUB:
        case INST_MUL:
        case INST_DIV:
        case INST_MOD:
            break;
        case INST_LT:
        case INST_LE:
        case INST_GT:
        case INST_GE:
        case INST_EQ:
        case INST_NE:
        case INST_AND:
        case INST_OR:
            break;
        default:
            assert(false && " should not reach here");
            break;
    }
    str.append(_operand1->getName()).append(" ").append(_operand2->getName());
    return str;
}

std::string JumpInst::toString() {
    std::string str = "jump";
    str.append(" ").append(_targetBB->getName());
    return str;
}

std::string CondJumpInst::toString() {
    std::string str;
    str.append("if").append(" ").append(_operand1->getName());
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

    str.append(_operand2->getName())
        .append(" ")
        .append("jump")
        .append(" ")
        .append(_trueBB->getName())
        .append(" ")
        .append("else")
        .append(" ")
        .append("jump")
        .append(" ")
        .append(_falseBB->getName());

    return str;
}

}  // namespace AtomIR
}  // namespace ATC