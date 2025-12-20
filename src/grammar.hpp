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

struct NodeStmtReturn {
    NodeExpr* expr;
};

struct NodeStmtLet {
    Token ident;
    NodeExpr* expr;
};

struct NodeStmt {
    std::variant<NodeStmtReturn*, NodeStmtLet*> stmt;
};

struct NodeProgram {
    std::vector<NodeStmt*> stmts;
};

struct NodeBinExprAdd {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprMulti {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprSub{
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprDiv{
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExpr {
    std::variant<NodeBinExprAdd*, NodeBinExprMulti*, NodeBinExprSub*, NodeBinExprDiv*> bin_expr;
};