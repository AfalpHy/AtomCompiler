#ifndef AST_BUILDER_VISITOR_H
#define AST_BUILDER_VISITOR_H

#include <iostream>

#include "../TreeNode.h"
#include "ATCBaseVisitor.h"

namespace ATC {
class ASTBuilder : public ATCBaseVisitor {
public:
    virtual antlrcpp::Any visitCompUnit(ATCParser::CompUnitContext *ctx) override;

    virtual antlrcpp::Any visitConstDecl(ATCParser::ConstDeclContext *ctx) override;

    virtual antlrcpp::Any visitVarDecl(ATCParser::VarDeclContext *ctx) override;

    virtual antlrcpp::Any visitVarDef(ATCParser::VarDefContext *ctx) override;

    virtual antlrcpp::Any visitInitVal(ATCParser::InitValContext *ctx) override;

    virtual antlrcpp::Any visitFunctionDef(ATCParser::FunctionDefContext *ctx) override;

    virtual antlrcpp::Any visitFuncFParams(ATCParser::FuncFParamsContext *ctx) override;

    virtual antlrcpp::Any visitFuncFParam(ATCParser::FuncFParamContext *ctx) override;

    virtual antlrcpp::Any visitBlock(ATCParser::BlockContext *ctx) override;

    virtual antlrcpp::Any visitStmt(ATCParser::StmtContext *ctx) override;

    virtual antlrcpp::Any visitVarRef(ATCParser::VarRefContext *ctx) override;

    virtual antlrcpp::Any visitPrimaryExpr(ATCParser::PrimaryExprContext *ctx) override;

    virtual antlrcpp::Any visitNumber(ATCParser::NumberContext *ctx) override;

    virtual antlrcpp::Any visitUnaryExpr(ATCParser::UnaryExprContext *ctx) override;

    virtual antlrcpp::Any visitFuncRParams(ATCParser::FuncRParamsContext *ctx) override;

    virtual antlrcpp::Any visitMulExpr(ATCParser::MulExprContext *ctx) override;

    virtual antlrcpp::Any visitAddExpr(ATCParser::AddExprContext *ctx) override;

    virtual antlrcpp::Any visitRelExpr(ATCParser::RelExprContext *ctx) override;

    virtual antlrcpp::Any visitEqExpr(ATCParser::EqExprContext *ctx) override;

    virtual antlrcpp::Any visitLAndExpr(ATCParser::LAndExprContext *ctx) override;

    virtual antlrcpp::Any visitLOrExpr(ATCParser::LOrExprContext *ctx) override;

    void setTokenStream(antlr4::CommonTokenStream *token) { _token = token; }

private:
    antlr4::CommonTokenStream *_token;
    // std::vector<TreeNode *> _astNodeStack;
    // std::vector<antlr4::ParserRuleContext *> _antlrNodeStack;
};
}  // namespace ATC

#endif