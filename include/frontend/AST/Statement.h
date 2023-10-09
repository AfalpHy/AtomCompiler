#ifndef STATEMENT_H
#define STATEMENT_H

#include <vector>

#include "Expression.h"
#include "TreeNode.h"
#include "Variable.h"

namespace ATC {
class Statement : public TreeNode {
public:
    Statement(/* args */) = default;
    virtual int getClassId() override { return ID_STATEMENT; }
};

class Block : public Statement {
public:
    Block(/* args */) = default;
    virtual int getClassId() override { return ID_BLOCK_STATEMENT; }

    void addDecl(Decl* decl) { _decls.push_back(decl); }
    void addStmt(Statement* stmt) { _stmts.push_back(stmt); }

    const std::vector<Decl*>& getDecls() { return _decls; }
    const std::vector<Statement*>& getStmts() { return _stmts; }

private:
    std::vector<Decl*> _decls;
    std::vector<Statement*> _stmts;
};

class AssignStatement : public Statement {
public:
    AssignStatement(/* args */) = default;
    virtual int getClassId() override { return ID_ASSIGN_STATEMENT; }

    void setVar(VarRef* varRef) { _varRef = varRef; }
    void setValue(Expression* value) { _value = value; }

    VarRef* getVar() { return _varRef; }
    Expression* getValue() { return _value; }

private:
    VarRef* _varRef;
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

    void setExpr(Expression* expr) { _expr = expr; }

    Expression* getExpr() { return _expr; }

private:
    Expression* _expr = nullptr;
};
}  // namespace ATC

#endif