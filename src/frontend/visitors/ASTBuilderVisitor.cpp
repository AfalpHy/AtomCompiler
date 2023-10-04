#include "ASTBuilderVisitor.h"

antlrcpp::Any ASTBuilderVisitor::visitNumber(ATCParser::NumberContext *ctx) {
    auto num = ctx->IntConst();
    std::cout << num->toString() << std::endl;
    return visitChildren(ctx);
}