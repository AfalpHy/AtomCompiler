#pragma once

#include <vector>

#include "Expression.h"
#include "Variable.h"

namespace ATC {
class Statement : public TreeNode {
public:
    Statement() = default;
};

class Block : public Statement {
public:
    Block() = default;

    virtual int getClassId() override { return ID_BLOCK; }

    const std::vector<TreeNode*>& getElements() { return _elements; }

    void addElement(TreeNode* element) { _elements.push_back(element); }

    ACCEPT

private:
    std::vector<TreeNode*> _elements;
};

class BlankStatement : public Statement {
public:
    BlankStatement() = default;

    virtual int getClassId() override { return ID_BLANK_STATEMENT; }
};

class AssignStatement : public Statement {
public:
    AssignStatement() = default;

    virtual int getClassId() override { return ID_ASSIGN_STATEMENT; }

    void setLval(Expression* lval) { _lval = lval; }
    void setRval(Expression* rval) { _rval = rval; }

    Expression* getLval() { return _lval; }
    Expression* getRval() { return _rval; }

    ACCEPT

private:
    Expression* _lval;
    Expression* _rval;
};

class IfStatement : public Statement {
public:
    IfStatement() = default;

    virtual int getClassId() override { return ID_IF_STATEMENT; }

    void setCond(Expression* cond) { _cond = cond; }
    void setStmt(Statement* stmt) { _stmt = stmt; }
    void setElseStmt(ElseStatement* stmt) { _elseStmt = stmt; }

    Expression* getCond() { return _cond; }
    Statement* getStmt() { return _stmt; }
    ElseStatement* getElseStmt() { return _elseStmt; }

    ACCEPT

private:
    Expression* _cond = nullptr;
    Statement* _stmt = nullptr;
    ElseStatement* _elseStmt = nullptr;
};

class ElseStatement : public Statement {
public:
    ElseStatement() = default;

    virtual int getClassId() override { return ID_ELSE_STATEMENT; }

    void setStmt(Statement* stmt) { _stmt = stmt; }

    Statement* getStmt() { return _stmt; }

    ACCEPT

private:
    Statement* _stmt = nullptr;
};

class WhileStatement : public Statement {
public:
    WhileStatement() = default;

    virtual int getClassId() override { return ID_WHILE_STATEMENT; }

    void setCond(Expression* cond) { _cond = cond; }
    void setStmt(Statement* stmt) { _stmt = stmt; }

    Expression* getCond() { return _cond; }
    Statement* getStmt() { return _stmt; }

    ACCEPT

private:
    Expression* _cond = nullptr;
    Statement* _stmt = nullptr;
};

class BreakStatement : public Statement {
public:
    BreakStatement() = default;

    virtual int getClassId() override { return ID_BREAK_STATEMENT; }

    ACCEPT
};

class ContinueStatement : public Statement {
public:
    ContinueStatement() = default;

    virtual int getClassId() override { return ID_CONTINUE_STATEMENT; }

    ACCEPT
};

class ReturnStatement : public Statement {
public:
    ReturnStatement() = default;

    virtual int getClassId() override { return ID_RETURN_STATEMENT; }

    void setExpr(Expression* expr) { _expr = expr; }

    Expression* getExpr() { return _expr; }

    ACCEPT

private:
    Expression* _expr = nullptr;
};

class OtherStatement : public Statement {
public:
    OtherStatement() = default;

    virtual int getClassId() override { return ID_OTHER_STATEMENT; }

    void setExpr(Expression* expr) { _expr = expr; }

    Expression* getExpr() { return _expr; }

    ACCEPT

private:
    Expression* _expr = nullptr;
};
}  // namespace ATC
