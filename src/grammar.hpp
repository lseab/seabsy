#include <variant>

#include "tokenization.hpp"


struct NodeTermIdent {
    Token ident;
};


struct NodeTermIntLit {
    Token int_lit;
};


struct NodeTerm {
    std::variant<NodeTermIntLit*, NodeTermIdent*> term;
};


struct NodeExpr {
    std::variant<NodeTerm*> expr;
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