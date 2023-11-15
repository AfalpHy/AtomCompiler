#include "AST/tools/ASTBuilder.h"

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
        arrayType->addDimension(ExpressionHandle::evaluateConstExpr(expr));
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
    // 取EOF前一个元素作为stop
    compUnit->setPosition(ctx->getStart(), _token->get(ctx->getStop()->getTokenIndex() - 1));

    CompUnit::AllCompUnits.push_back(compUnit);

    CurrentScope = new Scope();
    compUnit->setScope(CurrentScope);
    _lastNode = compUnit;
    for (size_t i = 0; i < ctx->children.size(); i++) {
        auto any = ctx->children[i]->accept(this);
        if (any.is<VarDecl *>()) {
            compUnit->addElement(any.as<VarDecl *>());
        } else if (any.is<FunctionDef *>()) {
            compUnit->addElement(any.as<FunctionDef *>());
        }
    }
    _lastNode = compUnit->getParent();

    return nullptr;
}

antlrcpp::Any ASTBuilder::visitCType(ATCParser::CTypeContext *ctx) {
    DataType *cType = nullptr;
    if (!ctx->Star().empty()) {
        cType = new PointerType(_lastNode);
        cType->setPosition(ctx->Star()[0]->getSymbol(), ctx->Star()[0]->getSymbol());
        PointerType *tmp = (PointerType *)cType;
        for (auto i = 1; i < ctx->Star().size(); i++) {
            auto deepPointer = new PointerType(tmp);
            deepPointer->setPosition(ctx->Star()[i]->getSymbol(), ctx->Star()[i]->getSymbol());
            tmp->setBaseDataType(deepPointer);
            tmp = deepPointer;
        }
        BasicType *basicType = new BasicType(tmp);
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
        cType = new BasicType(_lastNode);
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
    auto varDecl = new VarDecl(_lastNode);
    varDecl->setPosition(ctx->getStart(), ctx->getStop());

    _lastNode = varDecl;
    varDecl->setDataType(ctx->cType()->accept(this).as<DataType *>());
    for (auto varDef : ctx->varDef()) {
        auto var = varDef->accept(this).as<Variable *>();
        if (ctx->Const()) {
            var->setIsConst(true);
        }
        if (CurrentScope->getParent() == nullptr) {
            var->setIsGlobal(true);
        }
        varDecl->addVariable(var);
    }
    _lastNode = varDecl->getParent();

    return varDecl;
}

antlrcpp::Any ASTBuilder::visitVarDef(ATCParser::VarDefContext *ctx) {
    auto var = new Variable(_lastNode);
    var->setName(ctx->Ident()->getText());
    var->setPosition(ctx->Ident()->getSymbol(), ctx->Ident()->getSymbol());

    auto declType = static_cast<VarDecl *>(var->getParent())->getDataType();
    if (!ctx->expr().empty()) {
        auto arrayType = new ArrayType(var);
        arrayType->setPosition(ctx->LeftBracket().front()->getSymbol(), ctx->RightBracket().back()->getSymbol());

        _lastNode = arrayType;
        for (auto constExpr : ctx->expr()) {
            auto dimension = constExpr->accept(this).as<Expression *>();
            arrayType->addDimensionExpr(dimension);
        }
        _lastNode = arrayType->getParent();

        fixupArrayType(arrayType);
        arrayType->setBaseDataType(declType);
        var->setDataType(arrayType);
    } else {
        var->setDataType(declType);
    }

    _lastNode = var;
    if (ctx->initVal()) {
        var->setInitValue(ctx->initVal()->accept(this).as<Expression *>());
    }
    _lastNode = var->getParent();

    CurrentScope->insertVariable(var->getName(), var);

    return var;
}

antlrcpp::Any ASTBuilder::visitInitVal(ATCParser::InitValContext *ctx) {
    if (ctx->expr()) {
        return ctx->expr()->accept(this);
    }
    auto nestedExpr = new NestedExpression(_lastNode);
    nestedExpr->setPosition(ctx->getStart(), ctx->getStop());
    _lastNode = nestedExpr;
    for (auto element : ctx->initVal()) {
        nestedExpr->addElement(element->accept(this).as<Expression *>());
    }
    _lastNode = nestedExpr->getParent();

    return (Expression *)nestedExpr;
}

antlrcpp::Any ASTBuilder::visitFunctionDef(ATCParser::FunctionDefContext *ctx) {
    auto functionDef = new FunctionDef(_lastNode);
    functionDef->setName(ctx->Ident()->getText());
    functionDef->setPosition(ctx->getStart(), ctx->getStop());
    CurrentScope->insertFunctionDef(functionDef->getName(), functionDef);

    auto parentScope = CurrentScope;
    CurrentScope = new Scope();
    CurrentScope->setParent(parentScope);
    parentScope->addChild(CurrentScope);
    functionDef->setScope(CurrentScope);

    _lastNode = functionDef;
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
    _lastNode = functionDef->getParent();

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
    auto varDecl = new VarDecl(_lastNode);
    varDecl->setPosition(ctx->getStart(), ctx->getStop());
    _lastNode = varDecl;
    auto declType = ctx->cType()->accept(this).as<DataType *>();
    varDecl->setDataType(declType);

    auto var = new Variable(varDecl);
    var->setName(ctx->Ident()->getText());
    var->setPosition(ctx->Ident()->getSymbol(), ctx->Ident()->getSymbol());

    if (ctx->getStop()->getText() != ctx->Ident()->getText()) {
        auto pointerType = new PointerType(var);
        if (!ctx->expr().empty()) {
            auto arrayType = new ArrayType(pointerType);
            _lastNode = arrayType;
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
    _lastNode = varDecl->getParent();

    varDecl->addVariable(var);
    CurrentScope->insertVariable(var->getName(), var);
    return varDecl;
}

antlrcpp::Any ASTBuilder::visitBlock(ATCParser::BlockContext *ctx) {
    auto block = new Block(_lastNode);
    block->setPosition(ctx->getStart(), ctx->getStop());

    auto parentScope = CurrentScope;
    CurrentScope = new Scope();
    CurrentScope->setParent(parentScope);
    parentScope->addChild(CurrentScope);
    block->setScope(CurrentScope);

    _lastNode = block;
    for (size_t i = 0; i < ctx->children.size(); i++) {
        auto any = ctx->children[i]->accept(this);
        if (any.is<VarDecl *>()) {
            block->addElement(any.as<VarDecl *>());
        } else if (any.is<Statement *>()) {
            block->addElement(any.as<Statement *>());
        }
    }
    _lastNode = block->getParent();

    CurrentScope = parentScope;

    return (Statement *)block;
}

antlrcpp::Any ASTBuilder::visitStmt(ATCParser::StmtContext *ctx) {
    if (ctx->lval()) {
        auto stmt = new AssignStatement(_lastNode);
        stmt->setPosition(ctx->getStart(), ctx->getStop());

        _lastNode = stmt;
        auto lval = ctx->lval()->accept(this).as<Expression *>();
        stmt->setLval(lval);
        stmt->setRval(ctx->expr()->accept(this).as<Expression *>());
        _lastNode = stmt->getParent();

        return (Statement *)stmt;
    } else if (ctx->block()) {
        return ctx->block()->accept(this);
    } else if (ctx->If()) {
        auto stmt = new IfStatement(_lastNode);
        stmt->setPosition(ctx->getStart(), ctx->getStop());

        _lastNode = stmt;
        auto cond = ctx->expr()->accept(this).as<Expression *>();
        stmt->setCond(cond);
        stmt->setStmt(ctx->stmt(0)->accept(this).as<Statement *>());
        if (ctx->Else()) {
            auto elseStmt = new ElseStatement();
            elseStmt->setStmt(ctx->stmt(1)->accept(this).as<Statement *>());
            stmt->setElseStmt(elseStmt);
        }
        _lastNode = stmt->getParent();

        return (Statement *)stmt;
    } else if (ctx->While()) {
        auto stmt = new WhileStatement(_lastNode);
        stmt->setPosition(ctx->getStart(), ctx->getStop());

        _lastNode = stmt;
        auto cond = ctx->expr()->accept(this).as<Expression *>();
        stmt->setCond(cond);
        stmt->setStmt(ctx->stmt(0)->accept(this).as<Statement *>());
        _lastNode = stmt->getParent();

        return (Statement *)stmt;
    } else if (ctx->Break()) {
        auto stmt = new BreakStatement(_lastNode);
        stmt->setPosition(ctx->getStart(), ctx->getStop());
        return (Statement *)stmt;
    } else if (ctx->Continue()) {
        auto stmt = new ContinueStatement(_lastNode);
        stmt->setPosition(ctx->getStart(), ctx->getStop());
        return (Statement *)stmt;
    } else if (ctx->Return()) {
        auto stmt = new ReturnStatement(_lastNode);
        stmt->setPosition(ctx->getStart(), ctx->getStop());
        if (ctx->expr()) {
            _lastNode = stmt;
            stmt->setExpr(ctx->expr()->accept(this).as<Expression *>());
            _lastNode = stmt->getParent();
        }
        return (Statement *)stmt;
    } else if (ctx->expr()) {
        auto stmt = new OtherStatement(_lastNode);
        stmt->setPosition(ctx->getStart(), ctx->getStop());

        _lastNode = stmt;
        stmt->setExpr(ctx->expr()->accept(this).as<Expression *>());
        _lastNode = stmt->getParent();

        return (Statement *)stmt;
    } else {
        auto stmt = new BlankStatement(_lastNode);
        stmt->setPosition(ctx->getStart(), ctx->getStop());
        return (Statement *)stmt;
    }
}

antlrcpp::Any ASTBuilder::visitVarRef(ATCParser::VarRefContext *ctx) {
    auto varRef = new VarRef(_lastNode);
    varRef->setPosition(ctx->getStart(), ctx->getStop());
    varRef->setName(ctx->getStart()->getText());
    varRef->setVariable(CurrentScope->getVariable(varRef->getName()));
    return (Expression *)varRef;
}

antlrcpp::Any ASTBuilder::visitIndexedRef(ATCParser::IndexedRefContext *ctx) {
    auto indexedRef = new IndexedRef(_lastNode);
    indexedRef->setPosition(ctx->getStart(), ctx->getStop());
    indexedRef->setName(ctx->Ident()->getText());
    indexedRef->setVariable(CurrentScope->getVariable(indexedRef->getName()));

    _lastNode = indexedRef;
    for (auto dimension : ctx->expr()) {
        indexedRef->addDimension(dimension->accept(this).as<Expression *>());
    }
    _lastNode = indexedRef->getParent();

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
    auto constVal = new ConstVal(_lastNode);
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
        auto functionCall = new FunctionCall(_lastNode);
        functionCall->setName(ctx->Ident()->getText());
        if (ctx->funcRParams()) {
            _lastNode = functionCall;
            auto rParams = ctx->funcRParams()->accept(this).as<std::vector<Expression *>>();
            _lastNode = functionCall->getParent();

            for (auto rParam : rParams) {
                functionCall->addParams(rParam);
            }
        }
        functionCall->setFunctionDef(CurrentScope->getFunctionDef(functionCall->getName()));
        return (Expression *)functionCall;
    } else {
        auto unaryExpr = new UnaryExpression(_lastNode);
        auto unaryOp = ctx->unaryOp()->getText();
        if (unaryOp == "+") {
            unaryExpr->setOperator(PLUS);
        } else if (unaryOp == "-") {
            unaryExpr->setOperator(MINUS);
        } else {
            unaryExpr->setOperator(NOT);
        }

        _lastNode = unaryExpr;
        unaryExpr->setOperand(ctx->unaryExpr()->accept(this).as<Expression *>());
        _lastNode = unaryExpr->getParent();

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

        left->setParent(mulExpr);
        mulExpr->setLeft(left);

        auto right = unaryExprs[i]->accept(this).as<Expression *>();
        right->setParent(mulExpr);
        mulExpr->setRight(right);

        mulExpr->setPosition(ctx->getStart(), unaryExprs[i]->getStop());
        left = mulExpr;
    }
    left->setParent(_lastNode);
    left->setPosition(ctx->getStart(), ctx->getStop());
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
        left->setParent(addExpr);
        addExpr->setLeft(left);

        auto right = mulExprs[i]->accept(this).as<Expression *>();
        right->setParent(addExpr);
        addExpr->setRight(right);

        addExpr->setPosition(ctx->getStart(), mulExprs[i]->getStop());
        left = addExpr;
    }
    left->setParent(_lastNode);
    left->setPosition(ctx->getStart(), ctx->getStop());
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
        left->setParent(relExpr);
        relExpr->setLeft(left);

        auto right = addExprs[i]->accept(this).as<Expression *>();
        right->setParent(relExpr);
        relExpr->setRight(right);

        relExpr->setPosition(ctx->getStart(), addExprs[i]->getStop());
        left = relExpr;
    }
    left->setParent(_lastNode);
    left->setPosition(ctx->getStart(), ctx->getStop());
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
        left->setParent(eqExpr);
        eqExpr->setLeft(left);

        auto right = relExprs[i]->accept(this).as<Expression *>();
        right->setParent(eqExpr);
        eqExpr->setRight(right);

        eqExpr->setPosition(ctx->getStart(), relExprs[i]->getStop());
        left = eqExpr;
    }
    left->setParent(_lastNode);
    left->setPosition(ctx->getStart(), ctx->getStop());
    return (Expression *)left;
}

antlrcpp::Any ASTBuilder::visitLAndExpr(ATCParser::LAndExprContext *ctx) {
    auto eqExprs = ctx->eqExpr();

    auto left = eqExprs[0]->accept(this).as<Expression *>();
    for (int i = 1; i < eqExprs.size(); i++) {
        BinaryExpression *andExpr = new BinaryExpression();
        left->setParent(andExpr);
        andExpr->setOperator(AND);
        andExpr->setLeft(left);
        auto right = eqExprs[i]->accept(this).as<Expression *>();
        right->setParent(andExpr);
        andExpr->setRight(right);
        andExpr->setPosition(ctx->getStart(), eqExprs[i]->getStop());
        left = andExpr;
    }
    left->setParent(_lastNode);
    left->setPosition(ctx->getStart(), ctx->getStop());
    return (Expression *)left;
}

antlrcpp::Any ASTBuilder::visitLOrExpr(ATCParser::LOrExprContext *ctx) {
    auto lAndExprs = ctx->lAndExpr();

    auto left = lAndExprs[0]->accept(this).as<Expression *>();
    for (int i = 1; i < lAndExprs.size(); i++) {
        BinaryExpression *orExpr = new BinaryExpression();
        left->setParent(orExpr);
        orExpr->setOperator(OR);
        orExpr->setLeft(left);
        auto right = lAndExprs[i]->accept(this).as<Expression *>();
        right->setParent(orExpr);
        orExpr->setRight(right);
        orExpr->setPosition(ctx->getStart(), lAndExprs[i]->getStop());
        left = orExpr;
    }
    left->setParent(_lastNode);
    left->setPosition(ctx->getStart(), ctx->getStop());
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

antlrcpp::Any ASTBuilder::visitFloatConst(ATCParser::FloatConstContext *ctx) {
    if (ctx->DecFloatConst()) {
        return std::stof(ctx->getText());
    } else {
        return std::stof(ctx->getText());
    }
}
}  // namespace ATC
