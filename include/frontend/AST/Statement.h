#ifndef STATEMENT_H
#define STATEMENT_H

#include <vector>

#include "Expression.h"
#include "Node.h"
#include "Variable.h"

namespace ATC {
class Statement : public Node {
public:
    Statement(/* args */) = default;
    virtual int getClassId() override { return ID_STATEMENT; }
};

class Block : public Statement {
public:
    Block(/* args */) = default;
    virtual int getClassId() override { return ID_BLOCK_STATEMENT; }

    void addDecl(Node* decl) { _decls.push_back(decl); }
    void addStmt(Statement* stmt) { _stmts.push_back(stmt); }

    const std::vector<Node*>& getDecls() { return _decls; }
    const std::vector<Statement*>& getStmts() { return _stmts; }

private:
    std::vector<Node*> _decls;
    std::vector<Statement*> _stmts;
};

class AssignStatement : public Statement {
public:
    AssignStatement(/* args */) = default;
    virtual int getClassId() override { return ID_ASSIGN_STATEMENT; }

    void setVar(Variable* var) { _var = var; }
    void setValue(Expression* value) { _value = value; }

    Variable* getVar() { return _var; }
    Expression* getValue() { return _value; }

private:
    Variable* _var;
    Expression* _value;
};

class IfStatement : public Statement {
public:
    IfStatement(/* args */) = default;
    virtual int getClassId() override { return ID_IF_STATEMENT; }

    void setCond(Expression* cond) { _cond = cond; }
    void setStmt(Statement* stmt) { _stmt = stmt; }

    Expression* getCond() { return _cond; }
    Statement* getStmt() { return _stmt; }

private:
    Expression* _cond = nullptr;
    Statement* _stmt = nullptr;
};

class ReturnStatement : public Statement {
public:
    ReturnStatement(/* args */) = default;
    virtual int getClassId() override { return ID_RETURN_STATEMENT; }

    void setStmt(Statement* stmt) { _stmt = stmt; }

    Statement* getStmt() { return _stmt; }

private:
    Statement* _stmt = nullptr;
};
}  // namespace ATC

#endif