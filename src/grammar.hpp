#include <variant>

#include "tokenization.hpp"


struct NodeExprIdent {
    Token ident;
};


struct NodeExprIntLit {
    Token int_lit;
};


struct NodeExpr {
    std::variant<NodeExprIdent*, NodeExprIntLit*> expr;
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