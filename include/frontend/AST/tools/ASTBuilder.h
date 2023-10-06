#ifndef AST_BUILDER_VISITOR_H
#define AST_BUILDER_VISITOR_H

#include "ATCBaseVisitor.h"
#include <iostream>

class ASTBuilder : public ATCBaseVisitor {
  public:
    virtual antlrcpp::Any visitNumber(ATCParser::NumberContext *ctx) override;
};

#endif