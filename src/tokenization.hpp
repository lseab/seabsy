#pragma once

#include <iostream>
#include <optional>
#include <string>
#include <vector>


enum class TokenType {
    _return,
    int_lit,
    semi,
    ident,
};


struct Token {
    TokenType type;
    std::optional<std::string> value {};
};


class Tokenizer {
    public:
        inline Tokenizer(const std::string& src):
            m_src(src)
        {
        }
        inline std::vector<Token> tokenize() {
            std::vector<Token> tokens;
            std::string buffer;

            while (inspect().has_value()) {
                if (std::isalpha(inspect().value())) {
                    buffer.push_back(consume());
                    while (inspect().has_value() && std::isalnum(inspect().value())) {
                        buffer.push_back(consume());
                    }
                    if (buffer == "return") {
                        tokens.push_back(Token{.type = TokenType::_return});
                        buffer.clear();
                        continue;
                    }
                    else {
                        tokens.push_back(Token{.type = TokenType::ident, .value = buffer});
                        continue;
                    }
                }
                else if (std::isdigit(inspect().value())) {
                    buffer.push_back(consume());
                    while (inspect().has_value() && std::isdigit(inspect().value())) {
                        buffer.push_back(consume());
                    }
                    tokens.push_back(Token{.type = TokenType::int_lit, .value = buffer});
                    buffer.clear();
                }
                else if (inspect().value() == ';') {
                    consume();
                    tokens.push_back(Token{.type = TokenType::semi});
                }
                else if (std::isspace(inspect().value())) {
                    consume();
                    continue;
                }
            }

            m_index = 0;
            return tokens;
        }

    private:
        std::optional<char> inspect(int offset = 0) const
        {
            if (m_index + offset >= m_src.length()) return {};
            else return m_src.at(m_index + offset);
        }

        char consume() {
            char value = m_src.at(m_index);
            m_index++;
            return value;
        }

        const std::string m_src;
        size_t m_index = 0;
};
