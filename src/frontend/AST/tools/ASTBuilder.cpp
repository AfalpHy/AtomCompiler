#include "AST/tools/ASTBuilder.h"

namespace ATC {
antlrcpp::Any ASTBuilder::visitNumber(ATCParser::NumberContext *ctx) {
    auto num = ctx->IntConst();
    std::cout << num->toString() << std::endl;
    return visitChildren(ctx);
}
}  // namespace ATC
