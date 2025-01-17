#pragma once

#include <iostream>

#include "ATCBaseVisitor.h"
#include "TreeNode.h"

namespace ATC {
class ASTBuilder : public ATCBaseVisitor {
public:
    ASTBuilder(antlr4::CommonTokenStream *token) : _token(token) {}

    virtual antlrcpp::Any visitCompUnit(ATCParser::CompUnitContext *ctx) override;

    virtual antlrcpp::Any visitCType(ATCParser::CTypeContext *ctx) override;

    virtual antlrcpp::Any visitVarDecl(ATCParser::VarDeclContext *ctx) override;

    virtual antlrcpp::Any visitVarDef(ATCParser::VarDefContext *ctx) override;

    virtual antlrcpp::Any visitInitVal(ATCParser::InitValContext *ctx) override;

    virtual antlrcpp::Any visitFunctionDeclOrDef(ATCParser::FunctionDeclOrDefContext *ctx) override;

    virtual antlrcpp::Any visitFuncFParams(ATCParser::FuncFParamsContext *ctx) override;

    virtual antlrcpp::Any visitFuncFParam(ATCParser::FuncFParamContext *ctx) override;

    virtual antlrcpp::Any visitVarRef(ATCParser::VarRefContext *ctx) override;

    virtual antlrcpp::Any visitIndexedRef(ATCParser::IndexedRefContext *ctx) override;

    // virtual antlrcpp::Any visitDereference(ATCParser::DereferenceContext *ctx) override;

    virtual antlrcpp::Any visitNumber(ATCParser::NumberContext *ctx) override;

    virtual antlrcpp::Any visitPrimaryExpr(ATCParser::PrimaryExprContext *ctx) override;

    virtual antlrcpp::Any visitUnaryExpr(ATCParser::UnaryExprContext *ctx) override;

    virtual antlrcpp::Any visitFuncRParams(ATCParser::FuncRParamsContext *ctx) override;

    virtual antlrcpp::Any visitMulExpr(ATCParser::MulExprContext *ctx) override;

    virtual antlrcpp::Any visitAddExpr(ATCParser::AddExprContext *ctx) override;

    virtual antlrcpp::Any visitRelExpr(ATCParser::RelExprContext *ctx) override;

    virtual antlrcpp::Any visitEqExpr(ATCParser::EqExprContext *ctx) override;

    virtual antlrcpp::Any visitLAndExpr(ATCParser::LAndExprContext *ctx) override;

    virtual antlrcpp::Any visitLOrExpr(ATCParser::LOrExprContext *ctx) override;

    virtual antlrcpp::Any visitBlock(ATCParser::BlockContext *ctx) override;

    virtual antlrcpp::Any visitStmt(ATCParser::StmtContext *ctx) override;

    virtual antlrcpp::Any visitIntConst(ATCParser::IntConstContext *ctx) override;

    virtual antlrcpp::Any visitFloatConst(ATCParser::FloatConstContext *ctx) override;

private:
    antlr4::CommonTokenStream *_token;
};
}  // namespace ATC
