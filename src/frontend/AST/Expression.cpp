#include "AST/Expression.h"

#include "AST/FunctionDef.h"

namespace ATC {

static float getArrayElement(IndexedRef *indexedRef) {
    static std::unordered_map<Variable *, std::unordered_map<int, float>> var2arrayElemnt;
    Variable *var = indexedRef->getVariable();
    assert(var->isConst());
    Expression *initExpr = var->getInitValue();
    if (initExpr == nullptr) {
        return 0;
    }
    const auto &refDimensions = indexedRef->getDimensions();

    auto &elementSize = static_cast<ATC::ArrayType *>(var->getDataType())->getElementSize();

    int elementIndex = 0;
    for (int i = 0; i != refDimensions.size(); i++) {
        elementIndex += ExpressionHandle::evaluateConstExpr(refDimensions[i]) * elementSize[i];
    }
    if (var2arrayElemnt.find(var) != var2arrayElemnt.end()) {
        return var2arrayElemnt[var][elementIndex];
    }

    int deep = 0;
    int index = 0;
    auto dimensions = static_cast<ATC::ArrayType *>(var->getDataType())->getDimensions();
    int totalSize = static_cast<ATC::ArrayType *>(var->getDataType())->getTotalSize();
    std::function<void(NestedExpression *)> processNestExpr = [&](NestedExpression *nestExpr) {
        const auto &elements = nestExpr->getElements();
        int maxSize = deep > 0 ? elementSize[deep - 1] : totalSize;
        deep++;
        // get one value when there are excess elements in a scalar initializer or
        // when there are too many braces around a scalar initializer.
        if (index % maxSize || deep > dimensions.size()) {
            if (!elements.empty()) {
                if (elements[0]->getClassId() == ID_NESTED_EXPRESSION) {
                    processNestExpr((NestedExpression *)elements[0]);
                } else {
                    var2arrayElemnt[var][index] = ExpressionHandle::evaluateConstExpr(elements[0]);
                    index++;
                }
            }
        } else {
            int targetIndex = index + maxSize;
            for (size_t i = 0; i < elements.size(); i++) {
                if (elements[i]->getClassId() == ID_NESTED_EXPRESSION) {
                    processNestExpr((NestedExpression *)elements[i]);
                } else {
                    var2arrayElemnt[var][index] = ExpressionHandle::evaluateConstExpr(elements[i]);
                    index++;
                }
                // ignore the remaining elements
                if (index == targetIndex) {
                    break;
                }
            }
            index = targetIndex;
        }
        deep--;
    };

    processNestExpr((NestedExpression *)initExpr);

    return var2arrayElemnt[var][elementIndex];
}

float ExpressionHandle::evaluateConstExpr(Expression *expr) {
    assert(expr->isConst());
    switch (expr->getClassId()) {
        case ID_CONST_VAL: {
            auto constVal = (ConstVal *)expr;
            if (constVal->getBasicType() == BasicType::INT) {
                return constVal->getIntValue();
            } else {
                return constVal->getFloatValue();
            }
        }
        case ID_VAR_REF: {
            auto ret = evaluateConstExpr(((VarRef *)expr)->getVariable()->getInitValue());
            if (((VarRef *)expr)->getVariable()->getBasicType() == BasicType::INT) {
                ret = (int)ret;
            }
            return ret;
        }
        case ID_INDEXED_REF: {
            return getArrayElement((IndexedRef *)expr);
        }
        case ID_UNARY_EXPRESSION: {
            auto unaryExpr = (UnaryExpression *)expr;
            if (unaryExpr->getOperator() == NOT) {
                return !evaluateConstExpr(unaryExpr->getOperand());
            } else if (unaryExpr->getOperator() == MINUS) {
                return -evaluateConstExpr(unaryExpr->getOperand());
            } else {
                return evaluateConstExpr(unaryExpr->getOperand());
            }
        }
        default: {
            auto binaryExpr = (BinaryExpression *)expr;
            auto left = evaluateConstExpr(binaryExpr->getLeft());
            auto right = evaluateConstExpr(binaryExpr->getRight());
            switch (binaryExpr->getOperator()) {
                case PLUS:
                    return left + right;
                case MINUS:
                    return left - right;
                case MUL:
                    return left * right;
                case DIV:
                    return left / right;
                case MOD:
                    return (int)left % (int)right;
                case LT:
                    return left < right;
                case GT:
                    return left > right;
                case LE:
                    return left <= right;
                case GE:
                    return left >= right;
                case EQ:
                    return left == right;
                case NE:
                    return left != right;
                case AND:
                    return left && right;
                case OR:
                    return left || right;
                default:
                    assert(false && "could not reach here");
                    break;
            }
        }
    }
}

bool ExpressionHandle::isIntExpr(Expression *expr) {
    switch (expr->getClassId()) {
        case ID_CONST_VAL: {
            auto constVal = (ConstVal *)expr;
            return constVal->getBasicType() == BasicType::INT;
        }
        case ID_VAR_REF: {
            return ((VarRef *)expr)->getVariable()->getBasicType() == BasicType::INT;
        }
        case ID_INDEXED_REF: {
            return ((IndexedRef *)expr)->getVariable()->getBasicType() == BasicType::INT;
        }
        case ID_UNARY_EXPRESSION: {
            auto unaryExpr = (UnaryExpression *)expr;
            if (unaryExpr->getOperator() == NOT) {
                return true;
            } else if (unaryExpr->getOperator() == MINUS) {
                return isIntExpr(unaryExpr->getOperand());
            } else {
                return isIntExpr(unaryExpr->getOperand());
            }
        }
        default: {
            auto binaryExpr = (BinaryExpression *)expr;
            auto left = isIntExpr(binaryExpr->getLeft());
            auto right = isIntExpr(binaryExpr->getRight());
            switch (binaryExpr->getOperator()) {
                case PLUS:
                case MINUS:
                case MUL:
                case DIV:
                case MOD:
                    return left && right;
                case LT:
                case GT:
                case LE:
                case GE:
                case EQ:
                case NE:
                case AND:
                case OR:
                    return true;
                default:
                    assert(false && "could not reach here");
                    break;
            }
        }
    }
}

bool ExpressionHandle::isForValue(BinaryExpression *expr) {
    auto parent = expr->getParent();
    assert(parent && "the expr must have a parent node");
    switch (parent->getClassId()) {
        case ID_IF_STATEMENT:
        case ID_WHILE_STATEMENT:
            return false;
        case ID_BINARY_EXPRESSION: {
            BinaryExpression *parentExpr = (BinaryExpression *)parent;
            if (parentExpr->getOperator() == AND || parentExpr->getOperator() == OR) {
                return false;
            }
            break;
        }
        default:
            break;
    }
    return true;
}

}  // namespace ATC