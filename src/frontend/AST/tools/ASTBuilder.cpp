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

antlrcpp::Any ASTBuilder::visitCompUnit(ATCParser::CompUnitContext *ctx) {
    auto compUnit = new CompUnit();
    // 取EOF前一个元素作为stop
    compUnit->setPosition(ctx->getStart(), _token->get(ctx->getStop()->getTokenIndex() - 1));

    CompUnit::AllCompUnits.push_back(compUnit);
    CurrentScope = new Scope();
    compUnit->setScope(CurrentScope);
    for (size_t i = 0; i < ctx->children.size(); i++) {
        auto any = ctx->children[i]->accept(this);
        if (any.is<Decl *>()) {
            compUnit->addElement(any.as<Decl *>());
        } else if (any.is<FunctionDef *>()) {
            compUnit->addElement(any.as<FunctionDef *>());
        }
    }
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitVarDecl(ATCParser::VarDeclContext *ctx) {
    auto decl = new Decl();
    decl->setPosition(ctx->getStart(), ctx->getStop());

    for (auto varDef : ctx->varDef()) {
        auto var = varDef->accept(this).as<Variable *>();
        if (ctx->Const()) {
            var->setIsConst();
        }
        auto dataType = var->getDataType();
        if (ctx->Int()) {
            dataType->setBaseType(INT);
        } else {
            dataType->setBaseType(FLOAT);
        }
        var->setDataType(dataType);
        decl->addVariable(var);
    }

    return decl;
}

antlrcpp::Any ASTBuilder::visitVarDef(ATCParser::VarDefContext *ctx) {
    auto var = new Variable();
    var->setName(ctx->Ident()->getText());
    var->setPosition(ctx->Ident()->getSymbol(), ctx->Ident()->getSymbol());

    auto dataType = new DataType();
    var->setDataType(dataType);

    for (auto constExpr : ctx->expr()) {
        auto dimension = constExpr->accept(this).as<Expression *>();
        assert(dimension->isConst());
        dataType->addDimension(dimension);
    }

    if (ctx->initVal()) {
        var->setInitValue(ctx->initVal()->accept(this).as<Expression *>());
    }

    CurrentScope->insertVariable(var->getName(), var);
    return var;
}

antlrcpp::Any ASTBuilder::visitInitVal(ATCParser::InitValContext *ctx) {
    if (ctx->expr()) {
        return ctx->expr()->accept(this);
    }
    auto arrayExpr = new ArrayExpression();
    arrayExpr->setPosition(ctx->getStart(), ctx->getStop());
    for (auto element : ctx->initVal()) {
        arrayExpr->addElement(element->accept(this).as<Expression *>());
    }
    return (Expression *)arrayExpr;
}

antlrcpp::Any ASTBuilder::visitFunctionDef(ATCParser::FunctionDefContext *ctx) {
    auto functionDef = new FunctionDef();
    functionDef->setName(ctx->Ident()->getText());
    functionDef->setPosition(ctx->getStart(), ctx->getStop());

    if (ctx->Void()) {
        functionDef->setRetType(VOID);
    } else if (ctx->Int()) {
        functionDef->setRetType(INT);
    } else {
        functionDef->setRetType(FLOAT);
    }
    CurrentScope->insertFunctionDef(functionDef->getName(), functionDef);

    auto parentScope = CurrentScope;
    CurrentScope = new Scope();
    CurrentScope->setParent(parentScope);
    parentScope->addChild(CurrentScope);
    functionDef->setScope(CurrentScope);

    if (ctx->funcFParams()) {
        auto fParams = ctx->funcFParams()->accept(this).as<std::vector<Decl *>>();
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
    std::vector<Decl *> fParams;
    for (auto fParam : ctx->funcFParam()) {
        fParams.push_back(fParam->accept(this).as<Decl *>());
    }
    return fParams;
}

antlrcpp::Any ASTBuilder::visitFuncFParam(ATCParser::FuncFParamContext *ctx) {
    auto decl = new Decl();
    decl->setPosition(ctx->getStart(), ctx->getStop());
    auto dataType = new DataType();
    auto var = new Variable();
    var->setPosition(ctx->Ident()->getSymbol(), ctx->Ident()->getSymbol());
    var->setName(ctx->Ident()->getText());
    if (ctx->expr().size()) {
        auto baseDataType = new DataType();
        if (ctx->Int()) {
            baseDataType->setBaseType(INT);
        } else {
            baseDataType->setBaseType(FLOAT);
        }
        for (auto expr : ctx->expr()) {
            baseDataType->addDimension(expr->accept(this).as<Expression *>());
        }
        dataType->setBaseDataType(baseDataType);
    } else {
        if (ctx->Int()) {
            dataType->setBaseType(INT);
        } else {
            dataType->setBaseType(FLOAT);
        }
    }
    var->setDataType(dataType);
    decl->addVariable(var);
    CurrentScope->insertVariable(var->getName(), var);
    return decl;
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
        if (any.is<Decl *>()) {
            block->addElement(any.as<Decl *>());
        } else if (any.is<Statement *>()) {
            block->addElement(any.as<Statement *>());
        }
    }

    CurrentScope = parentScope;

    return (Statement *)block;
}

antlrcpp::Any ASTBuilder::visitStmt(ATCParser::StmtContext *ctx) {
    if (ctx->varRef()) {
        auto stmt = new AssignStatement();
        stmt->setPosition(ctx->getStart(), ctx->getStop());
        auto varRef = ctx->varRef()->accept(this).as<Expression *>();
        assert(varRef->getClassId() == ID_VAR_REF);
        stmt->setVar((VarRef *)varRef);
        stmt->setValue(ctx->expr()->accept(this).as<Expression *>());
        return (Statement *)stmt;
    } else if (ctx->block()) {
        return ctx->block()->accept(this);
    } else if (ctx->If()) {
        auto stmt = new IfStatement();
        stmt->setPosition(ctx->getStart(), ctx->getStop());
        stmt->setCond(ctx->cond()->accept(this).as<Expression *>());
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
        stmt->setCond(ctx->cond()->accept(this).as<Expression *>());
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
    for (auto expr : ctx->expr()) {
        varRef->addDimension(expr->accept(this).as<Expression *>());
    }
    CurrentScope->addNeedFixupNode(varRef);
    return (Expression *)varRef;
}

antlrcpp::Any ASTBuilder::visitPrimaryExpr(ATCParser::PrimaryExprContext *ctx) {
    if (!ctx->varRef() && !ctx->number()) {
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
    if (ctx->IntConst()) {
        constVal->setBaseType(INT);
        constVal->setIntValue(std::stoi(ctx->IntConst()->getText()));
    } else {
        constVal->setBaseType(FLOAT);
        constVal->setFloatValue(std::stof(ctx->FloatConst()->getText()));
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
        CurrentScope->addNeedFixupNode(functionCall);
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
    auto operators = ctx->MulDIV();

    auto left = unaryExprs[0]->accept(this).as<Expression *>();
    for (int i = 1; i < unaryExprs.size(); i++) {
        BinaryExpression *mulExpr = new BinaryExpression();

        // 表达式比操作符多1个
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

    left->setPosition(ctx->getStart(), ctx->getStop());
    return (Expression *)left;
}

antlrcpp::Any ASTBuilder::visitAddExpr(ATCParser::AddExprContext *ctx) {
    auto mulExprs = ctx->mulExpr();
    auto operators = ctx->PlusMinus();

    auto left = mulExprs[0]->accept(this).as<Expression *>();
    for (int i = 1; i < mulExprs.size(); i++) {
        BinaryExpression *addExpr = new BinaryExpression();

        // 表达式比操作符多1个
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

    left->setPosition(ctx->getStart(), ctx->getStop());
    return (Expression *)left;
}

antlrcpp::Any ASTBuilder::visitRelExpr(ATCParser::RelExprContext *ctx) {
    auto addExprs = ctx->addExpr();
    auto operators = ctx->Cmp();

    auto left = addExprs[0]->accept(this).as<Expression *>();
    for (int i = 1; i < addExprs.size(); i++) {
        BinaryExpression *relExpr = new BinaryExpression();

        // 表达式比操作符多1个
        if (operators[i - 1]->getText() == "+") {
            relExpr->setOperator(PLUS);
        } else {
            relExpr->setOperator(MINUS);
        }

        relExpr->setLeft(left);

        auto right = addExprs[i]->accept(this).as<Expression *>();
        relExpr->setRight(right);

        relExpr->setPosition(ctx->getStart(), addExprs[i]->getStop());
        left = relExpr;
    }

    left->setPosition(ctx->getStart(), ctx->getStop());
    return (Expression *)left;
}

antlrcpp::Any ASTBuilder::visitEqExpr(ATCParser::EqExprContext *ctx) {
    auto relExprs = ctx->relExpr();
    auto operators = ctx->EqNe();

    auto left = relExprs[0]->accept(this).as<Expression *>();
    for (int i = 1; i < relExprs.size(); i++) {
        BinaryExpression *eqExpr = new BinaryExpression();

        // 表达式比操作符多1个
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

    left->setPosition(ctx->getStart(), ctx->getStop());
    return (Expression *)left;
}

antlrcpp::Any ASTBuilder::visitLAndExpr(ATCParser::LAndExprContext *ctx) {
    auto eqExprs = ctx->eqExpr();

    auto left = eqExprs[0]->accept(this).as<Expression *>();
    for (int i = 1; i < eqExprs.size(); i++) {
        BinaryExpression *andExpr = new BinaryExpression();
        andExpr->setOperator(AND);
        andExpr->setLeft(left);
        auto right = eqExprs[i]->accept(this).as<Expression *>();
        andExpr->setRight(right);
        andExpr->setPosition(ctx->getStart(), eqExprs[i]->getStop());
        left = andExpr;
    }

    left->setPosition(ctx->getStart(), ctx->getStop());
    return (Expression *)left;
}

antlrcpp::Any ASTBuilder::visitLOrExpr(ATCParser::LOrExprContext *ctx) {
    auto lAndExprs = ctx->lAndExpr();

    auto left = lAndExprs[0]->accept(this).as<Expression *>();
    for (int i = 1; i < lAndExprs.size(); i++) {
        BinaryExpression *orExpr = new BinaryExpression();
        orExpr->setOperator(OR);
        orExpr->setLeft(left);
        auto right = lAndExprs[i]->accept(this).as<Expression *>();
        orExpr->setRight(right);
        orExpr->setPosition(ctx->getStart(), lAndExprs[i]->getStop());
        left = orExpr;
    }

    left->setPosition(ctx->getStart(), ctx->getStop());
    return (Expression *)left;
}

}  // namespace ATC
