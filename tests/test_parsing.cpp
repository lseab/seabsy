#include <catch2/catch_test_macros.hpp>

#include "../src/parsing.hpp"


template <typename T, typename N>
T* expectNode(N& node) {
    auto stmt_ptr = std::get_if<T*>(&node.variant);
    REQUIRE(stmt_ptr != nullptr);
    return *stmt_ptr;
}

std::optional<NodeProgram> parse_stmt(std::string prog_str) {
    Tokenizer tokenizer = Tokenizer(prog_str);
    std::vector<Token> tokens = tokenizer.tokenize();
    Parser parser(tokens);
    return parser.parse_program();
}

TEST_CASE("Parse return statement") {
    std::string prog_str = "return 1;";
    std::optional<NodeProgram> prog = parse_stmt(prog_str);
    REQUIRE(prog->stmts.size() == 1);
    auto node_return = expectNode<NodeStmtReturn>(*(prog->stmts[0]));
    auto node_term = expectNode<NodeTerm>(*(node_return->expr));
    auto node_int_lit = expectNode<NodeTermIntLit>(*node_term);
    REQUIRE(node_int_lit->int_lit.value == std::to_string(1));
}

TEST_CASE("Parse binary expression") {
    std::string prog_string = "return 1 + 2 - 3 * 4 / 5;";
    std::optional<NodeProgram> prog = parse_stmt(prog_string);
    REQUIRE(prog->stmts.size() == 1);
    auto node_return = expectNode<NodeStmtReturn>(*(prog->stmts[0]));
    auto node_expr = expectNode<NodeBinExpr>(*(node_return->expr));
    REQUIRE(node_expr->op.type == TokenType::minus);
    auto node_lhs = expectNode<NodeBinExpr>(*node_expr->lhs);
    REQUIRE(node_lhs->op.type == TokenType::plus);
    auto node_lhs_term1 = expectNode<NodeTerm>(*(node_lhs->lhs));
    auto node_lhs_int_lit1 = expectNode<NodeTermIntLit>(*node_lhs_term1);
    REQUIRE(node_lhs_int_lit1->int_lit.value == std::to_string(1));
    auto node_lhs_term2 = expectNode<NodeTerm>(*(node_lhs->rhs));
    auto node_lhs_int_lit2 = expectNode<NodeTermIntLit>(*node_lhs_term2);
    REQUIRE(node_lhs_int_lit2->int_lit.value == std::to_string(2));
    auto node_rhs = expectNode<NodeBinExpr>(*node_expr->rhs);
    REQUIRE(node_rhs->op.type == TokenType::fslash);
    auto node_rhs_expr1 = expectNode<NodeBinExpr>(*(node_rhs->lhs));
    REQUIRE(node_rhs_expr1->op.type == TokenType::star);
    auto node_rhs_term_expr1 = expectNode<NodeTerm>(*node_rhs_expr1->lhs);
    auto node_rhs_term_int_lit1 = expectNode<NodeTermIntLit>(*node_rhs_term_expr1);
    REQUIRE(node_rhs_term_int_lit1->int_lit.value == std::to_string(3));
    auto node_rhs_term_expr2 = expectNode<NodeTerm>(*node_rhs_expr1->rhs);
    auto node_rhs_term_int_lit2 = expectNode<NodeTermIntLit>(*node_rhs_term_expr2);
    REQUIRE(node_rhs_term_int_lit2->int_lit.value == std::to_string(4));
    auto node_last_term = expectNode<NodeTerm>(*node_rhs->rhs);
    auto node_last_term_intlit1 = expectNode<NodeTermIntLit>(*node_last_term);
    REQUIRE(node_last_term_intlit1->int_lit.value == std::to_string(5));
}

TEST_CASE("Parse let expression") {
    std::string prog_string = "let x = 5;";
    std::optional<NodeProgram> prog = parse_stmt(prog_string);
    REQUIRE(prog->stmts.size() == 1);
    auto node_let = expectNode<NodeStmtLet>(*(prog->stmts[0]));
    REQUIRE(node_let->ident.type == TokenType::ident);
    REQUIRE(node_let->ident.value.value() == "x");
    auto node_term = expectNode<NodeTerm>(*(node_let->expr));
    auto node_int_lit = expectNode<NodeTermIntLit>(*node_term);
    REQUIRE(node_int_lit->int_lit.value == std::to_string(5));
}

TEST_CASE("Parse exit") {
    std::string prog_string = "exit(1);";
    std::optional<NodeProgram> prog = parse_stmt(prog_string);
    REQUIRE(prog->stmts.size() == 1);
    auto node_exit = expectNode<NodeStmtExit>(*(prog->stmts[0]));
    auto node_term = expectNode<NodeTerm>(*(node_exit->expr));
    auto node_term_paren = expectNode<NodeTermParen>(*node_term);
    auto node_term2 = expectNode<NodeTerm>(*(node_term_paren->expr));
    auto node_int_lit = expectNode<NodeTermIntLit>(*node_term2);
    REQUIRE(node_int_lit->int_lit.value == std::to_string(1));
}

TEST_CASE("Parse scopes") {
    std::string prog_string = "{return 5;}";
    std::optional<NodeProgram> prog = parse_stmt(prog_string);
    REQUIRE(prog->stmts.size() == 1);
    auto node_scope = expectNode<NodeScope>(*(prog->stmts[0]));
    auto node_return = expectNode<NodeStmtReturn>(*(node_scope->stmts[0]));
    auto node_term = expectNode<NodeTerm>(*(node_return->expr));
    auto node_int_lit = expectNode<NodeTermIntLit>(*node_term);
    REQUIRE(node_int_lit->int_lit.value == std::to_string(5));
}

TEST_CASE("Parse nested scopes") {
    std::string prog_string = "{let x = 1; {return x;}}";
    std::optional<NodeProgram> prog = parse_stmt(prog_string);
    REQUIRE(prog->stmts.size() == 1);
    // Outer scope
    auto out_scope_node = expectNode<NodeScope>(*(prog->stmts[0]));
    REQUIRE(out_scope_node->stmts.size() == 2);
    // Inner scope
    auto inner_scope_node = expectNode<NodeScope>(*(out_scope_node->stmts[1]));
    REQUIRE(inner_scope_node->stmts.size() == 1);
    // return x
    auto node_return = expectNode<NodeStmtReturn>(*(inner_scope_node->stmts[0]));
    auto node_return_term = expectNode<NodeTerm>(*(node_return->expr));
    auto node_return_ident = expectNode<NodeTermIdent>(*node_return_term);
    REQUIRE(node_return_ident->ident.value.value() == "x");
    REQUIRE(node_return_ident->ident.type == TokenType::ident);
}

TEST_CASE("Parse if statement") {
    std::string prog_string = "if(0){return 0;} elif(1) {return 1;} else {return 2;}";
    std::optional<NodeProgram> prog = parse_stmt(prog_string);
    REQUIRE(prog->stmts.size() == 1);
    // IF statement
    auto node_if = expectNode<NodeStmtIf>(*(prog->stmts[0]));
    auto node_if_term = expectNode<NodeTerm>(*(node_if->expr));
    auto node_if_int_lit = expectNode<NodeTermIntLit>(*node_if_term);
    REQUIRE(node_if_int_lit->int_lit.value == std::to_string(0));
    auto node_if_return = expectNode<NodeStmtReturn>(*(node_if->scope->stmts[0]));
    auto node_if_return_term = expectNode<NodeTerm>(*(node_if_return->expr));
    auto node_if_return_int_lit = expectNode<NodeTermIntLit>(*node_if_return_term);
    REQUIRE(node_if_return_int_lit->int_lit.value == std::to_string(0));
    // ELIF statement
    auto node_elif = expectNode<NodeStmtIf>(*(node_if->pred.value()));
    auto node_elif_term = expectNode<NodeTerm>(*(node_elif->expr));
    auto node_elif_int_lit = expectNode<NodeTermIntLit>(*node_elif_term);
    REQUIRE(node_elif_int_lit->int_lit.value == std::to_string(1));
    auto node_elif_return = expectNode<NodeStmtReturn>(*(node_elif->scope->stmts[0]));
    auto node_elif_return_term = expectNode<NodeTerm>(*(node_elif_return->expr));
    auto node_elif_return_int_lit = expectNode<NodeTermIntLit>(*node_elif_return_term);
    REQUIRE(node_elif_return_int_lit->int_lit.value == std::to_string(1));
    // ELSE statement
    auto node_else = expectNode<NodeIfPredElse>(*(node_elif->pred.value()));
    auto node_else_return = expectNode<NodeStmtReturn>(*(node_else->scope->stmts[0]));
    auto node_else_return_term = expectNode<NodeTerm>(*(node_else_return->expr));
    auto node_else_return_int_lit = expectNode<NodeTermIntLit>(*node_else_return_term);
    REQUIRE(node_else_return_int_lit->int_lit.value == std::to_string(2));
}