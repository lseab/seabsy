#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "arena.hpp"
#include "grammar.hpp"


class Parser {
public:
    explicit Parser(const std::vector<Token> tokens);

    std::optional<NodeTerm*> parse_term();
    std::optional<NodeScope*> parse_scope();
    std::optional<NodeStmtIf*> parse_if_stmt();
    std::optional<NodeExpr*> parse_expr(int min_prec = 0);
    std::optional<NodeIfPred*> parse_if_predicate();
    std::optional<NodeStmt*> parse_stmt();
    std::optional<NodeProgram> parse_program();

private:
    std::optional<Token> inspect(int offset = 0) const;
    Token consume();

    Token try_consume(TokenType type, const std::string& error_msg);
    std::optional<Token> try_consume(TokenType type);

    const std::vector<Token> m_tokens;
    size_t m_index = 0;
    ArenaAllocator m_arena;
};
