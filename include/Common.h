#ifndef COMMON_H
#define COMMON_H

namespace ATC {
enum NodeType {
    ID_COMP_UNIT,
    ID_DATA_TYPE,
    ID_DECL,
    ID_FUNC_DEF,
    ID_VARIABLE,

    // expression
    ID_EXPRESSION,
    ID_CONST_VAL,
    ID_VAR_REF,
    ID_UNARY_EXPRESSION,
    ID_BINARY_EXPRESSION,
    ID_FUNC_CALL,

    // statement
    ID_STATEMENT,
    ID_BLOCK,
    ID_ASSIGN_STATEMENT,
    ID_BLANK_STATEMENT,
    ID_IF_STATEMENT,
    ID_ELSE_STATEMENT,
    ID_WHILE_STATEMENT,
    ID_BREAK_STATEMENT,
    ID_CONTINUE_STATEMENT,
    ID_RETURN_STATEMENT
};

enum Operator { PLUS, MINUS, NOT, ADD, SUB, MUL, DIV, MOD, LT, GT, LE, GE, EQ, NE, AND, OR };

enum BaseDataType { VOID, INT, FLOAT };
}  // namespace ATC
#endif