#include <catch2/catch_test_macros.hpp>

#include "../src/tokenization.hpp"


TEST_CASE("Tokenize return statement") {
    std::string stmt = "return 1;";
    Tokenizer tokenizer = Tokenizer(stmt);
    std::vector<Token> tokens = tokenizer.tokenize();
    REQUIRE(tokens.size() == 3);
    REQUIRE(tokens[0].type == TokenType::_return);
    REQUIRE(tokens[1].type == TokenType::int_lit);
    REQUIRE(tokens[1].value.value() == std::to_string(1));
    REQUIRE(tokens[2].type == TokenType::semi);
}

TEST_CASE("Tokenize binary expression") {
    std::string stmt = "1 + 2 - 3 * 4 / 5;";
    Tokenizer tokenizer = Tokenizer(stmt);
    std::vector<Token> tokens = tokenizer.tokenize();
    REQUIRE(tokens.size() == 10);
    REQUIRE(tokens[0].type == TokenType::int_lit);
    REQUIRE(tokens[1].type == TokenType::plus);
    REQUIRE(tokens[2].type == TokenType::int_lit);
    REQUIRE(tokens[3].type == TokenType::minus);
    REQUIRE(tokens[4].type == TokenType::int_lit);
    REQUIRE(tokens[5].type == TokenType::star);
    REQUIRE(tokens[6].type == TokenType::int_lit);
    REQUIRE(tokens[7].type == TokenType::fslash);
    REQUIRE(tokens[8].type == TokenType::int_lit);
    REQUIRE(tokens[9].type == TokenType::semi);
}

TEST_CASE("Tokenize let expression") {
    std::string stmt = "let x = 5;";
    Tokenizer tokenizer = Tokenizer(stmt);
    std::vector<Token> tokens = tokenizer.tokenize();
    REQUIRE(tokens.size() == 5);
    REQUIRE(tokens[0].type == TokenType::let);
    REQUIRE(tokens[1].type == TokenType::ident);
    REQUIRE(tokens[2].type == TokenType::eq);
    REQUIRE(tokens[3].type == TokenType::int_lit);
    REQUIRE(tokens[4].type == TokenType::semi);
}

TEST_CASE("Tokenize exit") {
    std::string stmt = "exit(1);";
    Tokenizer tokenizer = Tokenizer(stmt);
    std::vector<Token> tokens = tokenizer.tokenize();
    REQUIRE(tokens.size() == 5);
    REQUIRE(tokens[0].type == TokenType::_exit);
    REQUIRE(tokens[1].type == TokenType::left_paren);
    REQUIRE(tokens[2].type == TokenType::int_lit);
    REQUIRE(tokens[3].type == TokenType::right_paren);
    REQUIRE(tokens[4].type == TokenType::semi);
}

TEST_CASE("Tokenize scopes") {
    std::string stmt = "{return 5;}";
    Tokenizer tokenizer = Tokenizer(stmt);
    std::vector<Token> tokens = tokenizer.tokenize();
    REQUIRE(tokens.size() == 5);
    REQUIRE(tokens[0].type == TokenType::open_curly);
    REQUIRE(tokens[1].type == TokenType::_return);
    REQUIRE(tokens[2].type == TokenType::int_lit);
    REQUIRE(tokens[3].type == TokenType::semi);
    REQUIRE(tokens[4].type == TokenType::close_curly);
}

TEST_CASE("Tokenize if statement") {
    std::string stmt = "if(0){return 0;} elif(1) {return 1;} else {return 2;}";
    Tokenizer tokenizer = Tokenizer(stmt);
    std::vector<Token> tokens = tokenizer.tokenize();
    REQUIRE(tokens.size() == 24);
    REQUIRE(tokens[0].type == TokenType::_if);
    REQUIRE(tokens[1].type == TokenType::left_paren);
    REQUIRE(tokens[2].type == TokenType::int_lit);
    REQUIRE(tokens[3].type == TokenType::right_paren);
    REQUIRE(tokens[4].type == TokenType::open_curly);
    REQUIRE(tokens[5].type == TokenType::_return);
    REQUIRE(tokens[6].type == TokenType::int_lit);
    REQUIRE(tokens[7].type == TokenType::semi);
    REQUIRE(tokens[8].type == TokenType::close_curly);
    REQUIRE(tokens[9].type == TokenType::_elif);
    REQUIRE(tokens[10].type == TokenType::left_paren);
    REQUIRE(tokens[11].type == TokenType::int_lit);
    REQUIRE(tokens[12].type == TokenType::right_paren);
    REQUIRE(tokens[13].type == TokenType::open_curly);
    REQUIRE(tokens[14].type == TokenType::_return);
    REQUIRE(tokens[15].type == TokenType::int_lit);
    REQUIRE(tokens[16].type == TokenType::semi);
    REQUIRE(tokens[17].type == TokenType::close_curly);
    REQUIRE(tokens[18].type == TokenType::_else);
    REQUIRE(tokens[19].type == TokenType::open_curly);
    REQUIRE(tokens[20].type == TokenType::_return);
    REQUIRE(tokens[21].type == TokenType::int_lit);
    REQUIRE(tokens[22].type == TokenType::semi);
    REQUIRE(tokens[23].type == TokenType::close_curly);
}