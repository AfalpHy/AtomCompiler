#include "dumpSourceASTVisitor.h"

using namespace std;

antlrcpp::Any DumpSourceASTVisitor::visitConstDecl(ATCParser::ConstDeclContext *context) {
    cout << context->bType()->getText() << endl;
    return visitChildren(context);
}

// antlrcpp::Any DumpSourceASTVisitor::visitConstDef(ATCParser::ConstDefContext *context) {}

// antlrcpp::Any DumpSourceASTVisitor::visitConstInitVal(ATCParser::ConstInitValContext *context) {}

// antlrcpp::Any DumpSourceASTVisitor::visitVarDecl(ATCParser::VarDeclContext *context) {}

// antlrcpp::Any DumpSourceASTVisitor::visitVarDef(ATCParser::VarDefContext *context) {}

// antlrcpp::Any DumpSourceASTVisitor::visitInitVal(ATCParser::InitValContext *context) {}

// antlrcpp::Any DumpSourceASTVisitor::visitFuncDef(ATCParser::FuncDefContext *context) {}

// antlrcpp::Any DumpSourceASTVisitor::visitFuncFParams(ATCParser::FuncFParamsContext *context) {}

// antlrcpp::Any DumpSourceASTVisitor::visitFuncFParam(ATCParser::FuncFParamContext *context) {}

// antlrcpp::Any DumpSourceASTVisitor::visitBlock(ATCParser::BlockContext *context) {}

// antlrcpp::Any DumpSourceASTVisitor::visitBlockItem(ATCParser::BlockItemContext *context) {}

// antlrcpp::Any DumpSourceASTVisitor::visitStmt(ATCParser::StmtContext *context) {}

// antlrcpp::Any DumpSourceASTVisitor::visitExpr(ATCParser::ExprContext *context) {}

// antlrcpp::Any DumpSourceASTVisitor::visitCond(ATCParser::CondContext *context) {}

// antlrcpp::Any DumpSourceASTVisitor::visitLval(ATCParser::LvalContext *context) {}

// antlrcpp::Any DumpSourceASTVisitor::visitPrimaryExpr(ATCParser::PrimaryExprContext *context) {}

antlrcpp::Any DumpSourceASTVisitor::visitNumber(ATCParser::NumberContext *context) {}

// antlrcpp::Any DumpSourceASTVisitor::visitUnaryExpr(ATCParser::UnaryExprContext *context) {}

// antlrcpp::Any DumpSourceASTVisitor::visitUnaryOp(ATCParser::UnaryOpContext *context) {}

// antlrcpp::Any DumpSourceASTVisitor::visitFuncRParams(ATCParser::FuncRParamsContext *context) {}

// antlrcpp::Any DumpSourceASTVisitor::visitMulExpr(ATCParser::MulExprContext *context) {}

// antlrcpp::Any DumpSourceASTVisitor::visitAddExpr(ATCParser::AddExprContext *context) {}

// antlrcpp::Any DumpSourceASTVisitor::visitRelExpr(ATCParser::RelExprContext *context) {}

// antlrcpp::Any DumpSourceASTVisitor::visitEqExpr(ATCParser::EqExprContext *context) {}

// antlrcpp::Any DumpSourceASTVisitor::visitLAndExpr(ATCParser::LAndExprContext *context) {}

// antlrcpp::Any DumpSourceASTVisitor::visitLOrExpr(ATCParser::LOrExprContext *context) {}

// antlrcpp::Any DumpSourceASTVisitor::visitConstExpr(ATCParser::ConstExprContext *context) {}