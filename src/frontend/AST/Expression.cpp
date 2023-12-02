#include "AST/Expression.h"

#include "AST/FunctionDef.h"

namespace ATC {
float ExpressionHandle::evaluateConstExpr(Expression* expr) {
    assert(expr->isConst());
    switch (expr->getClassId()) {
        case ID_CONST_VAL: {
            auto constVal = (ConstVal*)expr;
            if (constVal->getBasicType() == BasicType::INT) {
                return constVal->getIntValue();
            } else {
                return constVal->getFloatValue();
            }
        }
        case ID_VAR_REF: {
            auto ret = evaluateConstExpr(((VarRef*)expr)->getVariable()->getInitValue());
            if (((VarRef*)expr)->getVariable()->getBasicType() == BasicType::INT) {
                ret = (int)ret;
            }
            return ret;
        }
        case ID_UNARY_EXPRESSION: {
            auto unaryExpr = (UnaryExpression*)expr;
            if (unaryExpr->getOperator() == NOT) {
                return !evaluateConstExpr(unaryExpr->getOperand());
            } else if (unaryExpr->getOperator() == MINUS) {
                return -evaluateConstExpr(unaryExpr->getOperand());
            } else {
                return evaluateConstExpr(unaryExpr->getOperand());
            }
        }
        default: {
            auto binaryExpr = (BinaryExpression*)expr;
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

bool ExpressionHandle::isIntExpr(Expression* expr) {
    switch (expr->getClassId()) {
        case ID_CONST_VAL: {
            auto constVal = (ConstVal*)expr;
            if (constVal->getBasicType() == BasicType::INT) {
                return true;
            } else {
                return false;
            }
        }
        case ID_VAR_REF: {
            auto ret = evaluateConstExpr(((VarRef*)expr)->getVariable()->getInitValue());
            if (((VarRef*)expr)->getVariable()->getBasicType() == BasicType::INT) {
                return true;
            }
            return false;
        }
        case ID_UNARY_EXPRESSION: {
            auto unaryExpr = (UnaryExpression*)expr;
            if (unaryExpr->getOperator() == NOT) {
                return true;
            } else if (unaryExpr->getOperator() == MINUS) {
                return isIntExpr(unaryExpr->getOperand());
            } else {
                return isIntExpr(unaryExpr->getOperand());
            }
        }
        default: {
            auto binaryExpr = (BinaryExpression*)expr;
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

bool NestedExpression::isConst() {
    for (auto expr : _elements) {
        if (!expr->isConst()) {
            return false;
        }
    }
    return true;
}

}  // namespace ATC