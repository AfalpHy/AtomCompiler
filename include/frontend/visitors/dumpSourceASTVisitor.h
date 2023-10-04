#ifndef DUMP_SOURCE_AST_VISITOR_H
#define DUMP_SOURCE_AST_VISITOR_H

#include "ATCBaseVisitor.h"
#include <iostream>

class DumpSourceASTVisitor : public ATCBaseVisitor {
  public:
    // virtual antlrcpp::Any visitCompUnit(ATCParser::CompUnitContext *context) override;

    // virtual antlrcpp::Any visitDecl(ATCParser::DeclContext *context) override;

    virtual antlrcpp::Any visitConstDecl(ATCParser::ConstDeclContext *context) override;

    // virtual antlrcpp::Any visitConstDef(ATCParser::ConstDefContext *context) override;

    // virtual antlrcpp::Any visitConstInitVal(ATCParser::ConstInitValContext *context) override;

    // virtual antlrcpp::Any visitVarDecl(ATCParser::VarDeclContext *context) override;

    // virtual antlrcpp::Any visitVarDef(ATCParser::VarDefContext *context) override;

    // virtual antlrcpp::Any visitInitVal(ATCParser::InitValContext *context) override;

    // virtual antlrcpp::Any visitFuncDef(ATCParser::FuncDefContext *context) override;

    // virtual antlrcpp::Any visitFuncFParams(ATCParser::FuncFParamsContext *context) override;

    // virtual antlrcpp::Any visitFuncFParam(ATCParser::FuncFParamContext *context) override;

    // virtual antlrcpp::Any visitBlock(ATCParser::BlockContext *context) override;

    // virtual antlrcpp::Any visitBlockItem(ATCParser::BlockItemContext *context) override;

    // virtual antlrcpp::Any visitStmt(ATCParser::StmtContext *context) override;

    // virtual antlrcpp::Any visitExpr(ATCParser::ExprContext *context) override;

    // virtual antlrcpp::Any visitCond(ATCParser::CondContext *context) override;

    // virtual antlrcpp::Any visitLval(ATCParser::LvalContext *context) override;

    // virtual antlrcpp::Any visitPrimaryExpr(ATCParser::PrimaryExprContext *context) override;

    // virtual antlrcpp::Any visitNumber(ATCParser::NumberContext *context) override;

    // virtual antlrcpp::Any visitUnaryExpr(ATCParser::UnaryExprContext *context) override;

    // virtual antlrcpp::Any visitUnaryOp(ATCParser::UnaryOpContext *context) override;

    // virtual antlrcpp::Any visitFuncRParams(ATCParser::FuncRParamsContext *context) override;

    // virtual antlrcpp::Any visitMulExpr(ATCParser::MulExprContext *context) override;

    // virtual antlrcpp::Any visitAddExpr(ATCParser::AddExprContext *context) override;

    // virtual antlrcpp::Any visitRelExpr(ATCParser::RelExprContext *context) override;

    // virtual antlrcpp::Any visitEqExpr(ATCParser::EqExprContext *context) override;

    // virtual antlrcpp::Any visitLAndExpr(ATCParser::LAndExprContext *context) override;

    // virtual antlrcpp::Any visitLOrExpr(ATCParser::LOrExprContext *context) override;

    // virtual antlrcpp::Any visitConstExpr(ATCParser::ConstExprContext *context) override;

    // virtual antlrcpp::Any visitFuncType(ATCParser::FuncTypeContext *context) override;

    // virtual antlrcpp::Any visitBType(ATCParser::BTypeContext *context) override;
};

#endif