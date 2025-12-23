#include <variant>

#include "tokenization.hpp"


struct NodeExpr;

struct NodeTermIdent {
    Token ident;
};

struct NodeTermIntLit {
    Token int_lit;
};

struct NodeTermParen {
    NodeExpr* expr;
};

struct NodeTerm {
    std::variant<NodeTermIntLit*, NodeTermIdent*, NodeTermParen*> term;
};

struct NodeBinExpr;

struct NodeExpr {
    std::variant<NodeTerm*, NodeBinExpr*> expr;
};

struct NodeStmt;

struct NodeStmtReturn {
    NodeExpr* expr;
};

struct NodeStmtExit {
    NodeExpr* expr;
};

struct NodeStmtLet {
    Token ident;
    NodeExpr* expr;
};

struct NodeScope {
    std::vector<NodeStmt*> stmts;
};

struct NodeStmtIf {
    NodeExpr* expr;
    NodeScope* scope;
};

struct NodeStmt {
    std::variant<NodeStmtReturn*, NodeStmtLet*, NodeScope*, NodeStmtIf*, NodeStmtExit*> stmt;
};

struct NodeProgram {
    std::vector<NodeStmt*> stmts;
};

struct NodeBinExpr {
    Token op;
    NodeExpr* lhs;
    NodeExpr* rhs;
};