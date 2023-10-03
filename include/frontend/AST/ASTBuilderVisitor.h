#ifndef AST_BUILDER_VISITOR_H
#define AST_BUILDER_VISITOR_H

#include "ATCBaseVisitor.h"
#include <iostream>

class ASTBuilderVisitor : public ATCBaseVisitor {
  public:
    virtual antlrcpp::Any visitNumber(ATCParser::NumberContext *ctx) override {
        auto num = ctx->IntConst();
        std::cout << num->toString() << std::endl;
        return visitChildren(ctx);
    }
};

#endif