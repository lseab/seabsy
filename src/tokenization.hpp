#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

enum class TokenType {
    _return,
    int_lit,
    semi,
    ident,
    let,
    eq,
    plus,
    minus,
    star,
    fslash,
    left_paren,
    right_paren,
    open_curly,
    close_curly,
    _if,
    _else,
    _elif,
    _exit,
};

std::optional<int> bin_prec(TokenType type);

struct Token {
    TokenType type;
    std::optional<std::string> value {};
};

class Tokenizer {
public:
    explicit Tokenizer(const std::string& src);
    std::vector<Token> tokenize();

private:
    std::optional<char> inspect(int offset = 0) const;
    char consume();

    const std::string m_src;
    size_t m_index = 0;
};
