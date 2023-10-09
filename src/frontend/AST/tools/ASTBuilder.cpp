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
    auto any = visitChildren(ctx);
    _astNodeStack.pop_back();
    _antlrNodeStack.pop_back();

    return any;
}

antlrcpp::Any ASTBuilder::visitDecl(ATCParser::DeclContext *ctx) {
    auto decl = new Decl();

    auto astParent = _astNodeStack.back();
    if (astParent->getClassId() == ID_COMP_UNIT) {
        auto compUnit = (CompUnit *)astParent;
        compUnit->addElement(decl);
    }

    _astNodeStack.push_back(decl);
    _antlrNodeStack.push_back(ctx);
    auto any = visitChildren(ctx);
    _astNodeStack.pop_back();
    _antlrNodeStack.pop_back();

    return any;
}

antlrcpp::Any ASTBuilder::visitConstDecl(ATCParser::ConstDeclContext *ctx) {
    return visitChildren(ctx);
}

//    antlrcpp::Any ASTBuilder::visitConstDef(ATCParser::ConstDefContext *ctx)  {
//     return visitChildren(ctx);
//   }

//    antlrcpp::Any ASTBuilder::visitConstInitVal(ATCParser::ConstInitValContext *ctx)  {
//     return visitChildren(ctx);
//   }

antlrcpp::Any ASTBuilder::visitVarDecl(ATCParser::VarDeclContext *ctx) {
    _antlrNodeStack.push_back(ctx);
    auto any = visitChildren(ctx);
    _antlrNodeStack.pop_back();

    return any;
}

antlrcpp::Any ASTBuilder::visitVarDef(ATCParser::VarDefContext *ctx) {
    Variable *var = new Variable();

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

//    antlrcpp::Any ASTBuilder::visitLval(ATCParser::LvalContext *ctx)  {
//     return visitChildren(ctx);
//   }

//    antlrcpp::Any ASTBuilder::visitPrimaryExpr(ATCParser::PrimaryExprContext *ctx)  {
//     return visitChildren(ctx);
//   }

//    antlrcpp::Any ASTBuilder::visitNumber(ATCParser::NumberContext *ctx)  {
//     return visitChildren(ctx);
//   }

//    antlrcpp::Any ASTBuilder::visitUnaryExpr(ATCParser::UnaryExprContext *ctx)  {
//     return visitChildren(ctx);
//   }

//    antlrcpp::Any ASTBuilder::visitUnaryOp(ATCParser::UnaryOpContext *ctx)  {
//     return visitChildren(ctx);
//   }

//    antlrcpp::Any ASTBuilder::visitFuncRParams(ATCParser::FuncRParamsContext *ctx)  {
//     return visitChildren(ctx);
//   }

//    antlrcpp::Any ASTBuilder::visitMulExpr(ATCParser::MulExprContext *ctx)  {
//     return visitChildren(ctx);
//   }

//    antlrcpp::Any ASTBuilder::visitAddExpr(ATCParser::AddExprContext *ctx)  {
//     return visitChildren(ctx);
//   }

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

//    antlrcpp::Any ASTBuilder::visitConstExpr(ATCParser::ConstExprContext *ctx)  {
//     return visitChildren(ctx);
//   }

//    antlrcpp::Any ASTBuilder::visitFuncType(ATCParser::FuncTypeContext *ctx)  {
//     return visitChildren(ctx);
//   }

//    antlrcpp::Any ASTBuilder::visitBType(ATCParser::BTypeContext *ctx)  {
//     return visitChildren(ctx);
//   }
}  // namespace ATC
