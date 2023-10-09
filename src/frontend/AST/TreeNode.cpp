#include "AST/TreeNode.h"

namespace ATC {
void TreeNode::setPosition(antlr4::Token* start, antlr4::Token* stop) {
    _position._fileName = start->getTokenSource()->getSourceName();
    _position._leftLine = start->getLine();
    _position._leftColumn = start->getCharPositionInLine() + 1;
    _position._rightLine = stop->getLine();
    _position._rightColumn = stop->getCharPositionInLine() + stop->getText().length() + 1;
}
}  // namespace ATC