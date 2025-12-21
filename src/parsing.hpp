#include <iostream>
#include "arena.hpp"
#include "grammar.hpp"


class Parser {
public:
    inline Parser(const std::vector<Token> tokens)
        : m_tokens(tokens)
        , m_arena(1024 * 1024 * 4) // 4mb
        {
        };

    inline std::optional<NodeTerm*> parse_term() {
        if (auto int_lit = try_consume(TokenType::int_lit)) {
            NodeTermIntLit* term_int_lit = m_arena.alloc<NodeTermIntLit>();
            term_int_lit->int_lit = int_lit.value();
            NodeTerm* term = m_arena.alloc<NodeTerm>();
            term->term = term_int_lit;
            return term;
        }
        else if (auto ident = try_consume(TokenType::ident)) {
            NodeTermIdent* term_ident = m_arena.alloc<NodeTermIdent>();
            term_ident->ident = ident.value();
            NodeTerm* term = m_arena.alloc<NodeTerm>();
            term->term = term_ident;
            return term;
        }
        else if (auto left_paren = try_consume(TokenType::left_paren)) {
            std::optional<NodeExpr*> expr = parse_expr();
            NodeTermParen* term_paren = m_arena.alloc<NodeTermParen>();
            term_paren->expr = expr.value();
            NodeTerm* term = m_arena.alloc<NodeTerm>();
            term->term = term_paren;
            try_consume(TokenType::right_paren, "Expected )");
            return term;
        }
        return {};
    };

    inline std::optional<NodeExpr*> parse_expr(int min_prec = 0) {
        auto term = parse_term();
        if (!term.has_value()) {
            return {};
        }
        auto term_expr = m_arena.alloc<NodeExpr>();
        term_expr->expr = term.value();

        while (true) {
            std::optional<Token> current_token = inspect();
            if (!current_token.has_value()){
                break;
            }
            auto prec = bin_prec(current_token->type);
            if (!prec.has_value() || prec < min_prec) {
                break;
            }

            consume();
            int next_min_prec = prec.value() + 1; // left commutative
            auto rhs_expr = parse_expr(next_min_prec);
            if (!rhs_expr.has_value()) {
                std::cerr << "Expected expression" << std::endl;
                exit(EXIT_FAILURE);
            }

            auto bin_expr = m_arena.alloc<NodeBinExpr>();
            auto lhs_expr = m_arena.alloc<NodeExpr>();;
            lhs_expr->expr = term_expr->expr;
            bin_expr->lhs = lhs_expr;
            bin_expr->rhs = rhs_expr.value();
            bin_expr->op = current_token.value();
            term_expr->expr = bin_expr;
        }
        return term_expr;
    };

    inline std::optional<NodeStmt*> parse_stmt() {
        if (try_consume(TokenType::_return)) {
            NodeStmt* stmt = m_arena.alloc<NodeStmt>();
            NodeStmtReturn* stmt_return = m_arena.alloc<NodeStmtReturn>();
            if (auto node_expr = parse_expr()) {
                stmt_return->expr = node_expr.value();
            }
            else {
                std::cerr << "Invalid expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            try_consume(TokenType::semi, "Expected ;");
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
            try_consume(TokenType::semi, "Expected ;");
            stmt->stmt = stmt_let;
            return stmt;
        }
        else if (try_consume(TokenType::open_curly)) {
            auto scope = m_arena.alloc<NodeStmtScope>();
            while (inspect().has_value() && inspect().value().type != TokenType::close_curly) {
                if (auto stmt = parse_stmt()) {
                    scope->stmts.push_back(stmt.value());
                }
                else {
                    std::cerr << "Invalid statment" << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
            try_consume(TokenType::close_curly, "Expected }");
            auto stmt = m_arena.alloc<NodeStmt>();
            stmt->stmt = scope;
            return stmt;
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

    inline Token try_consume(TokenType type, const std::string& error_msg) {
        if (inspect().has_value() && inspect().value().type == type) {
            return consume();
        }
        else {
            std::cerr << error_msg<< std::endl;
            exit(EXIT_FAILURE);
        }
    }

    inline std::optional<Token> try_consume(TokenType type) {
        if (inspect().has_value() && inspect().value().type == type) {
            return consume();
        }
        else {
            return {};
        }
    }

    const std::vector<Token> m_tokens;
    size_t m_index = 0;
    ArenaAllocator m_arena;
};