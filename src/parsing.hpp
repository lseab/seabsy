#include "arena.hpp"
#include "grammar.hpp"


class Parser {
public:
    inline Parser(const std::vector<Token> tokens)
        : m_tokens(tokens)
        , m_arena(1024 * 1024 * 4) // 4mb
        {
        };

    inline std::optional<NodeExpr*> parse_expr() {
        if (inspect().has_value()) {
            if (inspect().value().type == TokenType::int_lit) {
                NodeExprIntLit* expr_int_lit = m_arena.alloc<NodeExprIntLit>();
                expr_int_lit->int_lit = consume();
                NodeExpr* expr = m_arena.alloc<NodeExpr>();
                expr->expr = expr_int_lit;
                return expr;
            }
            else if (inspect().value().type == TokenType::ident) {
                NodeExprIdent* expr_ident = m_arena.alloc<NodeExprIdent>();
                expr_ident->ident = consume();
                NodeExpr* expr = m_arena.alloc<NodeExpr>();
                expr->expr = expr_ident;
                return expr;
            }
        }
        return {};
    };

    inline std::optional<NodeStmt*> parse_stmt() {
        if (inspect().has_value()) {
            if (inspect().value().type == TokenType::_return) {
                consume();
                NodeStmt* stmt = m_arena.alloc<NodeStmt>();
                NodeStmtReturn* stmt_return = m_arena.alloc<NodeStmtReturn>();
                if (auto node_expr = parse_expr()) {
                    stmt_return->expr = node_expr.value();
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
                stmt->stmt = stmt_return;
                return stmt;
            }
            else if (
                inspect().value().type == TokenType::let &&
                inspect(1).value().type == TokenType::ident &&
                inspect(2).value().type == TokenType::eq
            ) {
                NodeStmt* stmt = m_arena.alloc<NodeStmt>();
                NodeStmtLet* stmt_let = m_arena.alloc<NodeStmtLet>();
                // consume the let token
                consume();
                // consume the identifier
                stmt_let->ident = consume();
                // consume the = token
                consume();
                if (auto node_expr = parse_expr()) {
                    stmt_let->expr = node_expr.value();
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
                stmt->stmt = stmt_let;
                return stmt;
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
    ArenaAllocator m_arena;
};