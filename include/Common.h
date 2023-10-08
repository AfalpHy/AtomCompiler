#ifndef COMMON_H
#define COMMON_H

namespace ATC {
enum NodeType {
    ID_COMP_UNIT,
    ID_DECL,
    ID_FUNC_DEF,
    ID_VARIABLE,

    // expression
    ID_EXPRESSION,
    ID_UNARY_EXPRESSION,
    ID_BINARY_EXPRESSION,
    ID_FUNC_CALL,

    // statement
    ID_STATEMENT,
    ID_BLOCK_STATEMENT,
    ID_ASSIGN_STATEMENT,
    ID_BLANK_STATEMENT,
    ID_IF_STATEMENT,
    ID_ELSE_STATEMENT,
    ID_WHILE_STATEMENT,
    ID_BREAK_STATEMENT,
    ID_CONTINUE_STATEMENT,
    ID_RETURN_STATEMENT
};

enum Operator { ADD, SUB, NOT, MUL, DIV, MOD, LT, GT, LE, GE, EQ, NE, AND, OR };

enum DataType { VOID, INT, FLOAT };
}  // namespace ATC
#endif