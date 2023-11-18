#include "atomIR/IRBuilder.h"

namespace ATC {

namespace AtomIR {

void IRBuilder::visit(FunctionDef *) {}

void IRBuilder::visit(Variable *) {}

void IRBuilder::visit(ConstVal *) {}

void IRBuilder::visit(VarRef *) {}

void IRBuilder::visit(IndexedRef *) {}

void IRBuilder::visit(NestedExpression *) {}

void IRBuilder::visit(UnaryExpression *) {}

void IRBuilder::visit(BinaryExpression *) {}

void IRBuilder::visit(FunctionCall *) {}

void IRBuilder::visit(Block *) {}

void IRBuilder::visit(AssignStatement *) {}

void IRBuilder::visit(IfStatement *) {}

void IRBuilder::visit(WhileStatement *) {}

void IRBuilder::visit(BreakStatement *) {}

void IRBuilder::visit(ContinueStatement *) {}

void IRBuilder::visit(ReturnStatement *) {}

}  // namespace AtomIR
}  // namespace ATC