#pragma once

#include <optional>
#include <variant>
#include <vector>

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
    std::variant<NodeTermIntLit*, NodeTermIdent*, NodeTermParen*> variant;
};

struct NodeBinExpr;

struct NodeExpr {
    std::variant<NodeTerm*, NodeBinExpr*> variant;
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

struct NodeIfPredElse {
    NodeScope* scope;
};

struct NodeStmtIf;

struct NodeIfPred {
    std::variant<NodeStmtIf*, NodeIfPredElse*> variant;
};

struct NodeStmtIf {
    NodeExpr* expr;
    NodeScope* scope;
    std::optional<NodeIfPred*> pred;
};

struct NodeStmt {
    std::variant<NodeStmtReturn*, NodeStmtLet*, NodeScope*, NodeStmtIf*, NodeStmtExit*> variant;
};

struct NodeProgram {
    std::vector<NodeStmt*> stmts;
};

struct NodeBinExpr {
    Token op;
    NodeExpr* lhs;
    NodeExpr* rhs;
};
