#include "AST/ASTBuilder.h"

#include "AST/CompUnit.h"
#include "AST/Decl.h"
#include "AST/Expression.h"
#include "AST/FunctionDef.h"
#include "AST/Scope.h"
#include "AST/Statement.h"
#include "AST/Variable.h"

namespace ATC {

Scope *CurrentScope = nullptr;

std::vector<CompUnit *> CompUnit::AllCompUnits;

static void fixupArrayType(ArrayType *arrayType) {
    const auto &dimensionExprs = arrayType->getDimensionExprs();
    for (auto expr : dimensionExprs) {
        arrayType->addDimension(ExpressionHandle::evaluateConstIntExpr(expr));
    }
    const auto &dimensions = arrayType->getDimensions();
    std::vector<int> elementSize(dimensionExprs.size());
    int size = 1;
    for (int i = dimensions.size() - 1; i >= 0; i--) {
        elementSize[i] = size;
        size *= dimensions[i];
    }
    arrayType->setElementSize(elementSize);
    arrayType->setTotalSize(size);
}

antlrcpp::Any ASTBuilder::visitCompUnit(ATCParser::CompUnitContext *ctx) {
    auto compUnit = new CompUnit();
    // take the token before EOF as stop token
    compUnit->setPosition(ctx->getStart(), _token->get(ctx->getStop()->getTokenIndex() - 1));

    CompUnit::AllCompUnits.push_back(compUnit);

    CurrentScope = new Scope();
    compUnit->setScope(CurrentScope);

    for (size_t i = 0; i < ctx->children.size(); i++) {
        auto any = ctx->children[i]->accept(this);
        if (any.is<VarDecl *>()) {
            compUnit->addElement(any.as<VarDecl *>());
        } else if (any.is<FunctionDef *>()) {
            compUnit->addElement(any.as<FunctionDef *>());
        }
    }

    return nullptr;
}

antlrcpp::Any ASTBuilder::visitCType(ATCParser::CTypeContext *ctx) {
    DataType *cType = nullptr;
    if (!ctx->Star().empty()) {
        cType = new PointerType();
        cType->setPosition(ctx->Star()[0]->getSymbol(), ctx->Star()[0]->getSymbol());
        PointerType *tmp = (PointerType *)cType;
        for (auto i = 1; i < ctx->Star().size(); i++) {
            auto deepPointer = new PointerType();
            deepPointer->setPosition(ctx->Star()[i]->getSymbol(), ctx->Star()[i]->getSymbol());
            tmp->setBaseDataType(deepPointer);
            tmp = deepPointer;
        }
        BasicType *basicType = new BasicType();
        if (ctx->Int()) {
            basicType->setType(BasicType::Type::INT);
            basicType->setPosition(ctx->Int()->getSymbol(), ctx->Int()->getSymbol());
        } else if (ctx->Float()) {
            basicType->setType(BasicType::Type::FLOAT);
            basicType->setPosition(ctx->Float()->getSymbol(), ctx->Float()->getSymbol());
        } else {
            assert(false && "not supported");
        }
        tmp->setBaseDataType(basicType);
    } else {
        cType = new BasicType();
        if (ctx->Int()) {
            static_cast<BasicType *>(cType)->setType(BasicType::Type::INT);
            cType->setPosition(ctx->Int()->getSymbol(), ctx->Int()->getSymbol());
        } else if (ctx->Float()) {
            static_cast<BasicType *>(cType)->setType(BasicType::Type::FLOAT);
            cType->setPosition(ctx->Float()->getSymbol(), ctx->Float()->getSymbol());
        } else {
            static_cast<BasicType *>(cType)->setType(BasicType::Type::VOID);
            cType->setPosition(ctx->Void()->getSymbol(), ctx->Void()->getSymbol());
        }
    }
    return cType;
}

antlrcpp::Any ASTBuilder::visitVarDecl(ATCParser::VarDeclContext *ctx) {
    auto varDecl = new VarDecl();
    varDecl->setPosition(ctx->getStart(), ctx->getStop());
    auto declType = ctx->cType()->accept(this).as<DataType *>();
    varDecl->setDataType(declType);
    for (auto varDef : ctx->varDef()) {
        auto var = varDef->accept(this).as<Variable *>();
        if (ctx->Const()) {
            var->setIsConst(true);
        }
        if (CurrentScope->getParent() == nullptr) {
            var->setIsGlobal(true);
        }
        if (var->getDataType() == nullptr) {
            var->setDataType(declType);
        } else {
            static_cast<ArrayType *>(var->getDataType())->setBaseDataType(declType);
        }
        varDecl->addVariable(var);
    }

    return varDecl;
}

antlrcpp::Any ASTBuilder::visitVarDef(ATCParser::VarDefContext *ctx) {
    auto var = new Variable();
    var->setName(ctx->Ident()->getText());
    var->setPosition(ctx->Ident()->getSymbol(), ctx->Ident()->getSymbol());

    if (!ctx->expr().empty()) {
        auto arrayType = new ArrayType();
        arrayType->setPosition(ctx->LeftBracket().front()->getSymbol(), ctx->RightBracket().back()->getSymbol());

        for (auto constExpr : ctx->expr()) {
            auto dimension = constExpr->accept(this).as<Expression *>();
            arrayType->addDimensionExpr(dimension);
        }
        fixupArrayType(arrayType);
        var->setDataType(arrayType);
    }

    if (ctx->initVal()) {
        auto initVal = ctx->initVal()->accept(this).as<Expression *>();
        if (initVal->getClassId() == ID_NESTED_EXPRESSION) {
            static_cast<NestedExpression *>(initVal)->setVariable(var);
        }
        var->setInitValue(initVal);
    }

    CurrentScope->insertVariable(var->getName(), var);

    return var;
}

antlrcpp::Any ASTBuilder::visitInitVal(ATCParser::InitValContext *ctx) {
    if (ctx->expr()) {
        return ctx->expr()->accept(this);
    }
    auto nestedExpr = new NestedExpression();
    nestedExpr->setPosition(ctx->getStart(), ctx->getStop());

    for (auto element : ctx->initVal()) {
        nestedExpr->addElement(element->accept(this).as<Expression *>());
    }

    return (Expression *)nestedExpr;
}

antlrcpp::Any ASTBuilder::visitFunctionDef(ATCParser::FunctionDefContext *ctx) {
    auto functionDef = new FunctionDef();
    functionDef->setName(ctx->Ident()->getText());
    functionDef->setPosition(ctx->getStart(), ctx->getStop());
    CurrentScope->insertFunctionDef(functionDef->getName(), functionDef);

    auto parentScope = CurrentScope;
    CurrentScope = new Scope();
    CurrentScope->setParent(parentScope);
    parentScope->addChild(CurrentScope);
    functionDef->setScope(CurrentScope);

    functionDef->setRetType(ctx->cType()->accept(this).as<DataType *>());
    if (ctx->funcFParams()) {
        auto fParams = ctx->funcFParams()->accept(this).as<std::vector<VarDecl *>>();
        for (auto fParam : fParams) {
            functionDef->addParams(fParam);
        }
    }
    auto block = ctx->block()->accept(this).as<Statement *>();
    assert(block->getClassId() == ID_BLOCK);
    functionDef->setBlock((Block *)block);

    CurrentScope = parentScope;
    return functionDef;
}

antlrcpp::Any ASTBuilder::visitFuncFParams(ATCParser::FuncFParamsContext *ctx) {
    std::vector<VarDecl *> fParams;
    for (auto fParam : ctx->funcFParam()) {
        fParams.push_back(fParam->accept(this).as<VarDecl *>());
    }
    return fParams;
}

antlrcpp::Any ASTBuilder::visitFuncFParam(ATCParser::FuncFParamContext *ctx) {
    auto varDecl = new VarDecl();
    varDecl->setPosition(ctx->getStart(), ctx->getStop());

    auto declType = ctx->cType()->accept(this).as<DataType *>();
    varDecl->setDataType(declType);

    auto var = new Variable();
    var->setName(ctx->Ident()->getText());
    var->setPosition(ctx->Ident()->getSymbol(), ctx->Ident()->getSymbol());

    if (ctx->getStop()->getText() != ctx->Ident()->getText()) {
        auto pointerType = new PointerType();
        if (!ctx->expr().empty()) {
            auto arrayType = new ArrayType();
            for (auto expr : ctx->expr()) {
                arrayType->addDimensionExpr(expr->accept(this).as<Expression *>());
            }
            fixupArrayType(arrayType);
            arrayType->setBaseDataType(declType);
            pointerType->setBaseDataType(arrayType);
        } else {
            pointerType->setBaseDataType(declType);
        }
        var->setDataType(pointerType);
    } else {
        var->setDataType(declType);
    }

    varDecl->addVariable(var);
    CurrentScope->insertVariable(var->getName(), var);
    return varDecl;
}

antlrcpp::Any ASTBuilder::visitBlock(ATCParser::BlockContext *ctx) {
    auto block = new Block();
    block->setPosition(ctx->getStart(), ctx->getStop());

    auto parentScope = CurrentScope;
    CurrentScope = new Scope();
    CurrentScope->setParent(parentScope);
    parentScope->addChild(CurrentScope);
    block->setScope(CurrentScope);

    for (size_t i = 0; i < ctx->children.size(); i++) {
        auto any = ctx->children[i]->accept(this);
        if (any.is<VarDecl *>()) {
            block->addElement(any.as<VarDecl *>());
        } else if (any.is<Statement *>()) {
            block->addElement(any.as<Statement *>());
        }
    }

    CurrentScope = parentScope;

    return (Statement *)block;
}

antlrcpp::Any ASTBuilder::visitStmt(ATCParser::StmtContext *ctx) {
    if (ctx->lval()) {
        auto stmt = new AssignStatement();
        stmt->setPosition(ctx->getStart(), ctx->getStop());
        auto lval = ctx->lval()->accept(this).as<Expression *>();
        stmt->setLval(lval);
        stmt->setRval(ctx->expr()->accept(this).as<Expression *>());
        return (Statement *)stmt;
    } else if (ctx->block()) {
        return ctx->block()->accept(this);
    } else if (ctx->If()) {
        auto stmt = new IfStatement();
        stmt->setPosition(ctx->getStart(), ctx->getStop());
        auto cond = ctx->expr()->accept(this).as<Expression *>();
        if (cond->getClassId() == ID_BINARY_EXPRESSION) {
            static_cast<BinaryExpression *>(cond)->setNotForValue();
        }
        stmt->setCond(cond);
        stmt->setStmt(ctx->stmt(0)->accept(this).as<Statement *>());
        if (ctx->Else()) {
            auto elseStmt = new ElseStatement();
            elseStmt->setStmt(ctx->stmt(1)->accept(this).as<Statement *>());
            stmt->setElseStmt(elseStmt);
        }
        return (Statement *)stmt;
    } else if (ctx->While()) {
        auto stmt = new WhileStatement();
        stmt->setPosition(ctx->getStart(), ctx->getStop());
        auto cond = ctx->expr()->accept(this).as<Expression *>();
        if (cond->getClassId() == ID_BINARY_EXPRESSION) {
            static_cast<BinaryExpression *>(cond)->setNotForValue();
        }
        stmt->setCond(cond);
        stmt->setStmt(ctx->stmt(0)->accept(this).as<Statement *>());
        return (Statement *)stmt;
    } else if (ctx->Break()) {
        auto stmt = new BreakStatement();
        stmt->setPosition(ctx->getStart(), ctx->getStop());
        return (Statement *)stmt;
    } else if (ctx->Continue()) {
        auto stmt = new ContinueStatement();
        stmt->setPosition(ctx->getStart(), ctx->getStop());
        return (Statement *)stmt;
    } else if (ctx->Return()) {
        auto stmt = new ReturnStatement();
        stmt->setPosition(ctx->getStart(), ctx->getStop());
        if (ctx->expr()) {
            stmt->setExpr(ctx->expr()->accept(this).as<Expression *>());
        }
        return (Statement *)stmt;
    } else if (ctx->expr()) {
        auto stmt = new OtherStatement();
        stmt->setPosition(ctx->getStart(), ctx->getStop());
        stmt->setExpr(ctx->expr()->accept(this).as<Expression *>());
        return (Statement *)stmt;
    } else {
        auto stmt = new BlankStatement();
        stmt->setPosition(ctx->getStart(), ctx->getStop());
        return (Statement *)stmt;
    }
}

antlrcpp::Any ASTBuilder::visitVarRef(ATCParser::VarRefContext *ctx) {
    auto varRef = new VarRef();
    varRef->setPosition(ctx->getStart(), ctx->getStop());
    varRef->setName(ctx->getStart()->getText());
    varRef->setVariable(CurrentScope->getVariable(varRef->getName()));
    return (Expression *)varRef;
}

antlrcpp::Any ASTBuilder::visitIndexedRef(ATCParser::IndexedRefContext *ctx) {
    auto indexedRef = new IndexedRef();
    indexedRef->setPosition(ctx->getStart(), ctx->getStop());
    indexedRef->setName(ctx->Ident()->getText());
    indexedRef->setVariable(CurrentScope->getVariable(indexedRef->getName()));

    for (auto dimension : ctx->expr()) {
        indexedRef->addDimension(dimension->accept(this).as<Expression *>());
    }

    return (Expression *)indexedRef;
}

antlrcpp::Any ASTBuilder::visitPrimaryExpr(ATCParser::PrimaryExprContext *ctx) {
    if (ctx->LeftParenthesis()) {
        auto expr = ctx->expr()->accept(this).as<Expression *>();
        expr->setPosition(ctx->getStart(), ctx->getStop());
        return expr;
    } else {
        return visitChildren(ctx);
    }
}

antlrcpp::Any ASTBuilder::visitNumber(ATCParser::NumberContext *ctx) {
    auto constVal = new ConstVal();
    constVal->setPosition(ctx->getStart(), ctx->getStop());
    if (ctx->intConst()) {
        constVal->setBasicType(BasicType::INT);
        constVal->setIntValue(ctx->intConst()->accept(this).as<int>());
    } else {
        constVal->setBasicType(BasicType::FLOAT);
        constVal->setFloatValue(ctx->floatConst()->accept(this).as<float>());
    }

    return (Expression *)constVal;
}

antlrcpp::Any ASTBuilder::visitUnaryExpr(ATCParser::UnaryExprContext *ctx) {
    if (ctx->primaryExpr()) {
        return ctx->primaryExpr()->accept(this);
    } else if (ctx->Ident()) {
        auto functionCall = new FunctionCall();
        functionCall->setName(ctx->Ident()->getText());
        if (ctx->funcRParams()) {
            auto rParams = ctx->funcRParams()->accept(this).as<std::vector<Expression *>>();
            for (auto rParam : rParams) {
                functionCall->addParams(rParam);
            }
        }
        functionCall->setFunctionDef(CurrentScope->getFunctionDef(functionCall->getName()));
        return (Expression *)functionCall;
    } else {
        auto unaryExpr = new UnaryExpression();
        auto unaryOp = ctx->unaryOp()->getText();
        if (unaryOp == "+") {
            unaryExpr->setOperator(PLUS);
        } else if (unaryOp == "-") {
            unaryExpr->setOperator(MINUS);
        } else {
            unaryExpr->setOperator(NOT);
        }

        unaryExpr->setOperand(ctx->unaryExpr()->accept(this).as<Expression *>());

        unaryExpr->setPosition(ctx->getStart(), ctx->getStop());
        return (Expression *)unaryExpr;
    }
}

antlrcpp::Any ASTBuilder::visitFuncRParams(ATCParser::FuncRParamsContext *ctx) {
    std::vector<Expression *> rParams;
    for (auto rParam : ctx->expr()) {
        rParams.push_back(rParam->accept(this).as<Expression *>());
    }
    return rParams;
}

antlrcpp::Any ASTBuilder::visitMulExpr(ATCParser::MulExprContext *ctx) {
    auto unaryExprs = ctx->unaryExpr();
    auto operators = ctx->mulDiv();

    auto left = unaryExprs[0]->accept(this).as<Expression *>();
    for (int i = 1; i < unaryExprs.size(); i++) {
        BinaryExpression *mulExpr = new BinaryExpression();

        // Number of expressions is one more than the number of operators.
        if (operators[i - 1]->getText() == "*") {
            mulExpr->setOperator(MUL);
        } else if (operators[i - 1]->getText() == "/") {
            mulExpr->setOperator(DIV);
        } else {
            mulExpr->setOperator(MOD);
        }

        mulExpr->setLeft(left);

        auto right = unaryExprs[i]->accept(this).as<Expression *>();

        mulExpr->setRight(right);

        mulExpr->setPosition(ctx->getStart(), unaryExprs[i]->getStop());
        left = mulExpr;
    }

    return (Expression *)left;
}

antlrcpp::Any ASTBuilder::visitAddExpr(ATCParser::AddExprContext *ctx) {
    auto mulExprs = ctx->mulExpr();
    auto operators = ctx->PlusMinus();

    auto left = mulExprs[0]->accept(this).as<Expression *>();
    for (int i = 1; i < mulExprs.size(); i++) {
        BinaryExpression *addExpr = new BinaryExpression();

        // Number of expressions is one more than the number of operators.
        if (operators[i - 1]->getText() == "+") {
            addExpr->setOperator(PLUS);
        } else {
            addExpr->setOperator(MINUS);
        }

        addExpr->setLeft(left);

        auto right = mulExprs[i]->accept(this).as<Expression *>();
        addExpr->setRight(right);

        addExpr->setPosition(ctx->getStart(), mulExprs[i]->getStop());
        left = addExpr;
    }

    return (Expression *)left;
}

antlrcpp::Any ASTBuilder::visitRelExpr(ATCParser::RelExprContext *ctx) {
    auto addExprs = ctx->addExpr();
    auto operators = ctx->Cmp();

    auto left = addExprs[0]->accept(this).as<Expression *>();
    for (int i = 1; i < addExprs.size(); i++) {
        BinaryExpression *relExpr = new BinaryExpression();

        // Number of expressions is one more than the number of operators.
        if (operators[i - 1]->getText() == "<") {
            relExpr->setOperator(LT);
        } else if (operators[i - 1]->getText() == ">") {
            relExpr->setOperator(GT);
        } else if (operators[i - 1]->getText() == "<=") {
            relExpr->setOperator(LE);
        } else if (operators[i - 1]->getText() == ">=") {
            relExpr->setOperator(GE);
        }

        relExpr->setLeft(left);

        auto right = addExprs[i]->accept(this).as<Expression *>();

        relExpr->setRight(right);

        relExpr->setPosition(ctx->getStart(), addExprs[i]->getStop());
        left = relExpr;
    }

    return (Expression *)left;
}

antlrcpp::Any ASTBuilder::visitEqExpr(ATCParser::EqExprContext *ctx) {
    auto relExprs = ctx->relExpr();
    auto operators = ctx->EqNe();

    auto left = relExprs[0]->accept(this).as<Expression *>();
    for (int i = 1; i < relExprs.size(); i++) {
        BinaryExpression *eqExpr = new BinaryExpression();

        // Number of expressions is one more than the number of operators.
        if (operators[i - 1]->getText() == "==") {
            eqExpr->setOperator(EQ);
        } else {
            eqExpr->setOperator(NE);
        }

        eqExpr->setLeft(left);

        auto right = relExprs[i]->accept(this).as<Expression *>();
        eqExpr->setRight(right);

        eqExpr->setPosition(ctx->getStart(), relExprs[i]->getStop());
        left = eqExpr;
    }

    return (Expression *)left;
}

antlrcpp::Any ASTBuilder::visitLAndExpr(ATCParser::LAndExprContext *ctx) {
    auto eqExprs = ctx->eqExpr();

    auto left = eqExprs[0]->accept(this).as<Expression *>();
    for (int i = 1; i < eqExprs.size(); i++) {
        BinaryExpression *andExpr = new BinaryExpression();

        andExpr->setOperator(AND);

        if (left->getClassId() == ID_BINARY_EXPRESSION) {
            static_cast<BinaryExpression *>(left)->setNotForValue();
        }
        andExpr->setLeft(left);

        auto right = eqExprs[i]->accept(this).as<Expression *>();
        if (right->getClassId() == ID_BINARY_EXPRESSION) {
            static_cast<BinaryExpression *>(right)->setNotForValue();
        }
        andExpr->setRight(right);

        andExpr->setPosition(ctx->getStart(), eqExprs[i]->getStop());
        left = andExpr;
    }

    return (Expression *)left;
}

antlrcpp::Any ASTBuilder::visitLOrExpr(ATCParser::LOrExprContext *ctx) {
    auto lAndExprs = ctx->lAndExpr();

    auto left = lAndExprs[0]->accept(this).as<Expression *>();
    for (int i = 1; i < lAndExprs.size(); i++) {
        BinaryExpression *orExpr = new BinaryExpression();

        orExpr->setOperator(OR);

        if (left->getClassId() == ID_BINARY_EXPRESSION) {
            static_cast<BinaryExpression *>(left)->setNotForValue();
        }
        orExpr->setLeft(left);

        auto right = lAndExprs[i]->accept(this).as<Expression *>();
        if (right->getClassId() == ID_BINARY_EXPRESSION) {
            static_cast<BinaryExpression *>(right)->setNotForValue();
        }
        orExpr->setRight(right);

        orExpr->setPosition(ctx->getStart(), lAndExprs[i]->getStop());
        left = orExpr;
    }

    return (Expression *)left;
}

antlrcpp::Any ASTBuilder::visitIntConst(ATCParser::IntConstContext *ctx) {
    if (ctx->DecIntConst()) {
        return std::stoi(ctx->getText());
    } else if (ctx->OctIntConst()) {
        return std::stoi(ctx->getText(), 0, 8);
    } else {
        return std::stoi(ctx->getText(), 0, 16);
    }
}

antlrcpp::Any ASTBuilder::visitFloatConst(ATCParser::FloatConstContext *ctx) { return std::stof(ctx->getText()); }
}  // namespace ATC
