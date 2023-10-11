#include "AST/tools/ASTBuilder.h"

#include "AST/CompUnit.h"
#include "AST/Decl.h"
#include "AST/Expression.h"
#include "AST/FuncDef.h"
#include "AST/Statement.h"
#include "AST/Variable.h"
namespace ATC {

std::vector<CompUnit *> CompUnit::AllCompUnits;
// helper;
static void printPosition(Position position) {
    printf("%s <%d:%d-%d:%d>\n", position._fileName.c_str(), position._leftLine,
           position._leftColumn, position._rightLine, position._rightColumn);
}

antlrcpp::Any ASTBuilder::visitCompUnit(ATCParser::CompUnitContext *ctx) {
    auto compUnit = new CompUnit();
    // 取EOF前一个元素作为stop
    compUnit->setPosition(ctx->getStart(), _token->get(ctx->getStop()->getTokenIndex() - 1));
    // printPosition(compUnit->getPosition());
    CompUnit::AllCompUnits.push_back(compUnit);

    _astNodeStack.push_back(compUnit);
    _antlrNodeStack.push_back(ctx);
    visitChildren(ctx);
    _astNodeStack.pop_back();
    _antlrNodeStack.pop_back();

    return nullptr;
}

antlrcpp::Any ASTBuilder::visitDecl(ATCParser::DeclContext *ctx) {
    auto decl = new Decl();
    decl->setPosition(ctx->getStart(), ctx->getStop());

    auto astParent = _astNodeStack.back();
    if (astParent->getClassId() == ID_COMP_UNIT) {
        auto compUnit = (CompUnit *)astParent;
        compUnit->addElement(decl);
    }

    _astNodeStack.push_back(decl);
    _antlrNodeStack.push_back(ctx);
    visitChildren(ctx);
    _astNodeStack.pop_back();
    _antlrNodeStack.pop_back();

    return nullptr;
}

// antlrcpp::Any ASTBuilder::visitConstDecl(ATCParser::ConstDeclContext *ctx) {
//     _antlrNodeStack.push_back(ctx);
//     auto any = visitChildren(ctx);
//     _antlrNodeStack.pop_back();
//     return any;
// }

//    antlrcpp::Any ASTBuilder::visitConstDef(ATCParser::ConstDefContext *ctx)  {
//     return visitChildren(ctx);
//   }

//    antlrcpp::Any ASTBuilder::visitConstInitVal(ATCParser::ConstInitValContext *ctx)  {
//     return visitChildren(ctx);
//   }

antlrcpp::Any ASTBuilder::visitVarDecl(ATCParser::VarDeclContext *ctx) {
    _antlrNodeStack.push_back(ctx);
    visitChildren(ctx);
    _antlrNodeStack.pop_back();

    return nullptr;
}

antlrcpp::Any ASTBuilder::visitVarDef(ATCParser::VarDefContext *ctx) {
    Variable *var = new Variable();
    var->setPosition(ctx->getStart(), ctx->getStop());

    auto antlrParent = _antlrNodeStack.back();
    if (antlrParent->getRuleIndex() == ATCParser::RuleVarDecl) {
        auto varDeclCtx = (ATCParser::VarDeclContext *)antlrParent;
        if (varDeclCtx->bType()->getStart()->getText() == "int") {
            var->setDataType(INT);
        }
    }
    _astNodeStack.push_back(var);
    _antlrNodeStack.push_back(ctx);
    auto any = visitChildren(ctx);
    _astNodeStack.pop_back();
    _antlrNodeStack.pop_back();

    return any;
}

//    antlrcpp::Any ASTBuilder::visitInitVal(ATCParser::InitValContext *ctx)  {
//     return visitChildren(ctx);
//   }

antlrcpp::Any ASTBuilder::visitFuncDef(ATCParser::FuncDefContext *ctx) {
    auto funcDef = new FuncDef();
    funcDef->setPosition(ctx->getStart(), ctx->getStop());
    return visitChildren(ctx);
}

//    antlrcpp::Any ASTBuilder::visitFuncFParams(ATCParser::FuncFParamsContext *ctx)  {
//     return visitChildren(ctx);
//   }

antlrcpp::Any ASTBuilder::visitFuncFParam(ATCParser::FuncFParamContext *ctx) {
    return visitChildren(ctx);
}

//    antlrcpp::Any ASTBuilder::visitBlock(ATCParser::BlockContext *ctx)  {
//     return visitChildren(ctx);
//   }

//    antlrcpp::Any ASTBuilder::visitBlockItem(ATCParser::BlockItemContext *ctx)  {
//     return visitChildren(ctx);
//   }

//    antlrcpp::Any ASTBuilder::visitStmt(ATCParser::StmtContext *ctx)  {
//     return visitChildren(ctx);
//   }

//    antlrcpp::Any ASTBuilder::visitExpr(ATCParser::ExprContext *ctx)  {
//     return visitChildren(ctx);
//   }

//    antlrcpp::Any ASTBuilder::visitCond(ATCParser::CondContext *ctx)  {
//     return visitChildren(ctx);
//   }

antlrcpp::Any ASTBuilder::visitLval(ATCParser::LvalContext *ctx) {
    VarRef *varRef = new VarRef();
    varRef->setPosition(ctx->getStart(), ctx->getStop());
    Variable *var = new Variable();
    var->setPosition(ctx->Ident()->getSymbol(), ctx->Ident()->getSymbol());
    varRef->setVariable(var);
    for (auto expr : ctx->expr()) {
        varRef->addDimension(expr->accept(this).as<Expression *>());
    }
    return varRef;
}

antlrcpp::Any ASTBuilder::visitPrimaryExpr(ATCParser::PrimaryExprContext *ctx) {
    auto any = visitChildren(ctx);
    if (ctx->lval() || ctx->number()) {
        return any;
    } else {
        auto expr = any.as<Expression *>();
        expr->setPosition(ctx->getStart(), ctx->getStop());
        return expr;
    }
}

antlrcpp::Any ASTBuilder::visitNumber(ATCParser::NumberContext *ctx) {
    ConstVal *constVal = new ConstVal();
    constVal->setPosition(ctx->getStart(), ctx->getStop());
    if (ctx->IntConst()) {
        constVal->setDataType(INT);
        constVal->setIntValue(std::stoi(ctx->IntConst()->getText()));
    } else {
        constVal->setDataType(FLOAT);
        constVal->setFloatValue(std::stof(ctx->IntConst()->getText()));
    }
    return constVal;
}

antlrcpp::Any ASTBuilder::visitUnaryExpr(ATCParser::UnaryExprContext *ctx) {
    if (ctx->primaryExpr()) {
        return ctx->primaryExpr()->accept(this);
    } else if (ctx->Ident()) {
        // funcall
    } else {
        auto unaryExpr = new UnaryExpression();
        auto unaryOp = ctx->unaryOp()->accept(this).as<std::string>();
        if (unaryOp == "+") {
            unaryExpr->setOperator(PLUS);
        } else if (unaryOp == "-") {
            unaryExpr->setOperator(MINUS);
        } else {
            unaryExpr->setOperator(NOT);
        }
        unaryExpr->setOperand(ctx->unaryExpr()->accept(this).as<Expression *>());
        unaryExpr->setPosition(ctx->getStart(), ctx->getStop());
        return unaryExpr;
    }
}

//    antlrcpp::Any ASTBuilder::visitUnaryOp(ATCParser::UnaryOpContext *ctx)  {
//     return visitChildren(ctx);
//   }

//    antlrcpp::Any ASTBuilder::visitFuncRParams(ATCParser::FuncRParamsContext *ctx)  {
//     return visitChildren(ctx);
//   }

antlrcpp::Any ASTBuilder::visitMulExpr(ATCParser::MulExprContext *ctx) {
    auto unaryExprs = ctx->unaryExpr();
    auto mulDivs = ctx->MulDIV();

    _antlrNodeStack.push_back(ctx);
    auto left = unaryExprs[0]->accept(this).as<Expression *>();
    for (int i = 1; i < unaryExprs.size(); i++) {
        BinaryExpression *mulExpr = new BinaryExpression();

        // 表达式比操作符多1个
        if (mulDivs[i - 1]->getText() == "*") {
            mulExpr->setOperator(ADD);
        } else if (mulDivs[i - 1]->getText() == "/") {
            mulExpr->setOperator(DIV);
        } else {
            mulExpr->setOperator(MOD);
        }

        mulExpr->setLeft(left);

        auto right = unaryExprs[i]->accept(this).as<Expression *>();
        mulExpr->setRight(right);

        left = mulExpr;
    }
    _antlrNodeStack.pop_back();

    left->setPosition(ctx->getStart(), ctx->getStop());
    return left;
}

antlrcpp::Any ASTBuilder::visitAddExpr(ATCParser::AddExprContext *ctx) {
    auto mulExprs = ctx->mulExpr();
    auto plusMinus = ctx->PlusMinus();

    _antlrNodeStack.push_back(ctx);
    auto left = mulExprs[0]->accept(this).as<Expression *>();
    for (int i = 1; i < mulExprs.size(); i++) {
        BinaryExpression *addExpr = new BinaryExpression();

        // 表达式比操作符多1个
        if (plusMinus[i - 1]->getText() == "+") {
            addExpr->setOperator(ADD);
        } else {
            addExpr->setOperator(SUB);
        }

        addExpr->setLeft(left);

        auto right = mulExprs[i]->accept(this).as<Expression *>();
        addExpr->setRight(right);

        left = addExpr;
    }
    _antlrNodeStack.pop_back();

    left->setPosition(ctx->getStart(), ctx->getStop());
    return left;
}

//    antlrcpp::Any ASTBuilder::visitRelExpr(ATCParser::RelExprContext *ctx)  {
//     return visitChildren(ctx);
//   }

//    antlrcpp::Any ASTBuilder::visitEqExpr(ATCParser::EqExprContext *ctx)  {
//     return visitChildren(ctx);
//   }

//    antlrcpp::Any ASTBuilder::visitLAndExpr(ATCParser::LAndExprContext *ctx)  {
//     return visitChildren(ctx);
//   }

//    antlrcpp::Any ASTBuilder::visitLOrExpr(ATCParser::LOrExprContext *ctx)  {
//     return visitChildren(ctx);
//   }

antlrcpp::Any ASTBuilder::visitConstExpr(ATCParser::ConstExprContext *ctx) {
    _antlrNodeStack.push_back(ctx);
    auto any = visitChildren(ctx);
    _antlrNodeStack.pop_back();
    return any;
}

//    antlrcpp::Any ASTBuilder::visitFuncType(ATCParser::FuncTypeContext *ctx)  {
//     return visitChildren(ctx);
//   }

//    antlrcpp::Any ASTBuilder::visitBType(ATCParser::BTypeContext *ctx)  {
//     return visitChildren(ctx);
//   }
}  // namespace ATC
