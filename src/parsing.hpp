#include <variant>

#include "tokenization.hpp"


struct NodeExprIdent {
    Token ident;
};


struct NodeExprIntLit {
    Token int_lit;
};


struct NodeExpr {
    std::variant<NodeExprIdent, NodeExprIntLit> expr;
};


struct NodeStmtReturn {
    NodeExpr expr;
};


struct NodeStmtLet {
    Token ident;
    NodeExpr expr;
};


struct NodeStmt {
    std::variant<NodeStmtReturn, NodeStmtLet> stmt;
};


struct NodeProgram {
    std::vector<NodeStmt> stmts;
};


class Parser {
public:
    inline Parser(const std::vector<Token> tokens):
        m_tokens(tokens)
        {
        };

    inline std::optional<NodeExpr> parse_expr() {
        if (inspect().has_value()) {
            if (inspect().value().type == TokenType::int_lit) {
                return NodeExpr{.expr = NodeExprIntLit{.int_lit = consume()}};
            }
            else if (inspect().value().type == TokenType::ident) {
                return NodeExpr{.expr = NodeExprIdent{.ident = consume()}};
            }
        }
        return {};
    };

    inline std::optional<NodeStmt> parse_stmt() {
        if (inspect().has_value()) {
            if (inspect().value().type == TokenType::_return) {
                consume();
                NodeStmtReturn stmt_return;
                if (auto node_expr = parse_expr()) {
                    stmt_return = {.expr = node_expr.value()};
                }
                else {
                    std::cerr << "Inalid expression" << std::endl;
                    exit(EXIT_FAILURE);
                }
                if (inspect().has_value() && inspect().value().type == TokenType::semi) {
                    consume();
                }
                else {
                    std::cerr << "Expected ;" << std::endl;
                    exit(EXIT_FAILURE);
                }
                return NodeStmt{.stmt = stmt_return};
            }
            else if (
                inspect().value().type == TokenType::let &&
                inspect(1).value().type == TokenType::ident &&
                inspect(2).value().type == TokenType::eq
            ) {
                // consume the let token
                consume();
                auto stmt_let = NodeStmtLet{.ident = consume()};
                // consume the = token
                consume();
                if (auto node_expr = parse_expr()) {
                    stmt_let.expr = node_expr.value();
                }
                else {
                    std::cerr << "Expected expression after let" << std::endl;
                    exit(EXIT_FAILURE);
                }
                if (inspect().has_value() && inspect().value().type == TokenType::semi) {
                    consume();
                }
                else {
                    std::cerr << "Expected ;" << std::endl;
                }
                return NodeStmt{.stmt = stmt_let};
            }
        }
        return {};
    };

    inline std::optional<NodeProgram> parse_program() {
        NodeProgram prog;
        while (inspect().has_value()) {
            if (auto stmt = parse_stmt()) {
                prog.stmts.push_back(stmt.value());
            }
            else {
                std::cerr << "Invalid statment" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        return prog;
    }

private:
    std::optional<Token> inspect(int offset = 0) const
    {
        if (m_index + offset >= m_tokens.size()) return {};
        else return m_tokens.at(m_index + offset);
    }

    Token consume() {
        Token value = m_tokens.at(m_index);
        m_index++;
        return value;
    }

    const std::vector<Token> m_tokens;
    size_t m_index = 0;
};