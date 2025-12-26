#include <iostream>

#include "parsing.hpp"


Parser::Parser(const std::vector<Token> tokens)
    : m_tokens(tokens)
    , m_arena(1024 * 1024 * 4)
{
}

std::optional<NodeTerm*> Parser::parse_term() {
    if (auto int_lit = try_consume(TokenType::int_lit)) {
        NodeTermIntLit* term_int_lit = m_arena.alloc<NodeTermIntLit>();
        term_int_lit->int_lit = int_lit.value();
        NodeTerm* term = m_arena.alloc<NodeTerm>();
        term->term = term_int_lit;
        return term;
    }
    if (auto ident = try_consume(TokenType::ident)) {
        NodeTermIdent* term_ident = m_arena.alloc<NodeTermIdent>();
        term_ident->ident = ident.value();
        NodeTerm* term = m_arena.alloc<NodeTerm>();
        term->term = term_ident;
        return term;
    }
    if (try_consume(TokenType::left_paren)) {
        std::optional<NodeExpr*> expr = parse_expr();
        NodeTermParen* term_paren = m_arena.alloc<NodeTermParen>();
        term_paren->expr = expr.value();
        NodeTerm* term = m_arena.alloc<NodeTerm>();
        term->term = term_paren;
        try_consume(TokenType::right_paren, "Expected )");
        return term;
    }
    return {};
}

std::optional<NodeScope*> Parser::parse_scope() {
    try_consume(TokenType::open_curly, "Expected {");
    auto scope = m_arena.alloc<NodeScope>();
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
    return scope;
}

std::optional<NodeStmtIf*> Parser::parse_if_stmt() {
    try_consume(TokenType::left_paren, "Expected (");
    NodeStmtIf* stmt_if = m_arena.alloc<NodeStmtIf>();
    if (auto expr = parse_expr()) {
        stmt_if->expr = expr.value();
    }
    else {
        std::cerr << "Invalid expression" << std::endl;
        exit(EXIT_FAILURE);
    }
    try_consume(TokenType::right_paren, "Expected )");
    if (auto scope = parse_scope()) {
        stmt_if->scope = scope.value();
    }
    else {
        std::cerr << "Invalid scope" << std::endl;
        exit(EXIT_FAILURE);
    }
    stmt_if->pred = parse_if_predicate();
    return stmt_if;
}

std::optional<NodeExpr*> Parser::parse_expr(int min_prec) {
    auto term = parse_term();
    if (!term.has_value()) {
        return {};
    }
    auto term_expr = m_arena.alloc<NodeExpr>();
    term_expr->expr = term.value();

    while (true) {
        std::optional<Token> current_token = inspect();
        if (!current_token.has_value()) {
            break;
        }
        auto prec = bin_prec(current_token->type);
        if (!prec.has_value() || prec < min_prec) {
            break;
        }

        consume();
        int next_min_prec = prec.value() + 1;
        auto rhs_expr = parse_expr(next_min_prec);
        if (!rhs_expr.has_value()) {
            std::cerr << "Expected expression" << std::endl;
            exit(EXIT_FAILURE);
        }

        auto bin_expr = m_arena.alloc<NodeBinExpr>();
        auto lhs_expr = m_arena.alloc<NodeExpr>();
        lhs_expr->expr = term_expr->expr;
        bin_expr->lhs = lhs_expr;
        bin_expr->rhs = rhs_expr.value();
        bin_expr->op = current_token.value();
        term_expr->expr = bin_expr;
    }
    return term_expr;
}

std::optional<NodeIfPred*> Parser::parse_if_predicate() {
    if (try_consume(TokenType::_elif)) {
        NodeIfPred* ifpred = m_arena.alloc<NodeIfPred>();
        auto stmt_if = parse_if_stmt();
        ifpred->ifpred = stmt_if.value();
        return ifpred;
    }
    if (try_consume(TokenType::_else)) {
        NodeIfPred* ifpred = m_arena.alloc<NodeIfPred>();
        NodeIfPredElse* ifpred_else = m_arena.alloc<NodeIfPredElse>();
        if (auto scope = parse_scope()) {
            ifpred_else->scope = scope.value();
        }
        else {
            std::cerr << "Invalid scope" << std::endl;
            exit(EXIT_FAILURE);
        }
        ifpred->ifpred = ifpred_else;
        return ifpred;
    }
    return {};
}

std::optional<NodeStmt*> Parser::parse_stmt() {
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
    if (try_consume(TokenType::_exit)) {
        NodeStmt* stmt = m_arena.alloc<NodeStmt>();
        NodeStmtExit* stmt_exit = m_arena.alloc<NodeStmtExit>();
        if (auto node_expr = parse_expr()) {
            stmt_exit->expr = node_expr.value();
        }
        else {
            std::cerr << "Invalid expression" << std::endl;
            exit(EXIT_FAILURE);
        }
        try_consume(TokenType::semi, "Expected ;");
        stmt->stmt = stmt_exit;
        return stmt;
    }
    if (
        inspect().value().type == TokenType::let &&
        inspect(1).value().type == TokenType::ident &&
        inspect(2).value().type == TokenType::eq
    ) {
        NodeStmt* stmt = m_arena.alloc<NodeStmt>();
        NodeStmtLet* stmt_let = m_arena.alloc<NodeStmtLet>();
        consume();
        stmt_let->ident = consume();
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
    if (try_consume(TokenType::open_curly)) {
        auto scope = parse_scope();
        auto stmt = m_arena.alloc<NodeStmt>();
        stmt->stmt = scope.value();
        return stmt;
    }
    if (try_consume(TokenType::_if)) {
        auto stmt_if = parse_if_stmt();
        NodeStmt* stmt = m_arena.alloc<NodeStmt>();
        stmt->stmt = stmt_if.value();
        return stmt;
    }
    return {};
}

std::optional<NodeProgram> Parser::parse_program() {
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

std::optional<Token> Parser::inspect(int offset) const {
    if (m_index + offset >= m_tokens.size()) return {};
    return m_tokens.at(m_index + offset);
}

Token Parser::consume() {
    Token value = m_tokens.at(m_index);
    m_index++;
    return value;
}

Token Parser::try_consume(TokenType type, const std::string& error_msg) {
    if (inspect().has_value() && inspect().value().type == type) {
        return consume();
    }
    std::cerr << error_msg << std::endl;
    exit(EXIT_FAILURE);
}

std::optional<Token> Parser::try_consume(TokenType type) {
    if (inspect().has_value() && inspect().value().type == type) {
        return consume();
    }
    return {};
}
