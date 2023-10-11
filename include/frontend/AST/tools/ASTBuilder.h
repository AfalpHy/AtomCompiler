#ifndef AST_BUILDER_VISITOR_H
#define AST_BUILDER_VISITOR_H

#include <iostream>

#include "../TreeNode.h"
#include "ATCBaseVisitor.h"

namespace ATC {
class ASTBuilder : public ATCBaseVisitor {
public:
    virtual antlrcpp::Any visitCompUnit(ATCParser::CompUnitContext *ctx) override;

    virtual antlrcpp::Any visitDecl(ATCParser::DeclContext *ctx) override;

    // virtual antlrcpp::Any visitConstDecl(ATCParser::ConstDeclContext *ctx) override;

    //   virtual antlrcpp::Any visitConstDef(ATCParser::ConstDefContext *ctx) override {
    //     return visitChildren(ctx);
    //   }

    //   virtual antlrcpp::Any visitConstInitVal(ATCParser::ConstInitValContext *ctx) override {
    //     return visitChildren(ctx);
    //   }

    virtual antlrcpp::Any visitVarDecl(ATCParser::VarDeclContext *ctx) override;

    virtual antlrcpp::Any visitVarDef(ATCParser::VarDefContext *ctx) override;

    //   virtual antlrcpp::Any visitInitVal(ATCParser::InitValContext *ctx) override {
    //     return visitChildren(ctx);
    //   }

    virtual antlrcpp::Any visitFuncDef(ATCParser::FuncDefContext *ctx) override;

    //   virtual antlrcpp::Any visitFuncFParams(ATCParser::FuncFParamsContext *ctx) override {
    //     return visitChildren(ctx);
    //   }

    virtual antlrcpp::Any visitFuncFParam(ATCParser::FuncFParamContext *ctx) override;

    //   virtual antlrcpp::Any visitBlock(ATCParser::BlockContext *ctx) override {
    //     return visitChildren(ctx);
    //   }

    //   virtual antlrcpp::Any visitBlockItem(ATCParser::BlockItemContext *ctx) override {
    //     return visitChildren(ctx);
    //   }

    //   virtual antlrcpp::Any visitStmt(ATCParser::StmtContext *ctx) override {
    //     return visitChildren(ctx);
    //   }

    // virtual antlrcpp::Any visitExpr(ATCParser::ExprContext *ctx) override;

    //   virtual antlrcpp::Any visitCond(ATCParser::CondContext *ctx) override {
    //     return visitChildren(ctx);
    //   }

    virtual antlrcpp::Any visitLval(ATCParser::LvalContext *ctx) override;

    virtual antlrcpp::Any visitPrimaryExpr(ATCParser::PrimaryExprContext *ctx) override;

    virtual antlrcpp::Any visitNumber(ATCParser::NumberContext *ctx) override;
    virtual antlrcpp::Any visitUnaryExpr(ATCParser::UnaryExprContext *ctx) override;

    //   virtual antlrcpp::Any visitUnaryOp(ATCParser::UnaryOpContext *ctx) override {
    //     return visitChildren(ctx);
    //   }

    //   virtual antlrcpp::Any visitFuncRParams(ATCParser::FuncRParamsContext *ctx) override {
    //     return visitChildren(ctx);
    //   }

    virtual antlrcpp::Any visitMulExpr(ATCParser::MulExprContext *ctx) override;

    virtual antlrcpp::Any visitAddExpr(ATCParser::AddExprContext *ctx) override;

    // virtual antlrcpp::Any visitRelExpr(ATCParser::RelExprContext *ctx) override;

    // virtual antlrcpp::Any visitEqExpr(ATCParser::EqExprContext *ctx) override;

    // virtual antlrcpp::Any visitLAndExpr(ATCParser::LAndExprContext *ctx) override;

    // virtual antlrcpp::Any visitLOrExpr(ATCParser::LOrExprContext *ctx) override;

    virtual antlrcpp::Any visitConstExpr(ATCParser::ConstExprContext *ctx) override;

    //   virtual antlrcpp::Any visitFuncType(ATCParser::FuncTypeContext *ctx) override {
    //     return visitChildren(ctx);
    //   }

    //   virtual antlrcpp::Any visitBType(ATCParser::BTypeContext *ctx) override {
    //     return visitChildren(ctx);
    //   }

    void setTokenStream(antlr4::CommonTokenStream *token) { _token = token; }

private:
    antlr4::CommonTokenStream *_token;
    std::vector<TreeNode *> _astNodeStack;
    std::vector<antlr4::ParserRuleContext *> _antlrNodeStack;
};
}  // namespace ATC

#endif