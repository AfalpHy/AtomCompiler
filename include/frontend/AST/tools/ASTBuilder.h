#ifndef AST_BUILDER_VISITOR_H
#define AST_BUILDER_VISITOR_H

#include <iostream>

#include "ATCBaseVisitor.h"
#include "../Node.h"
namespace ATC {
class ASTBuilder : public ATCBaseVisitor {
public:
    virtual antlrcpp::Any visitNumber(ATCParser::NumberContext *ctx) override;
};
}  // namespace ATC

#endif