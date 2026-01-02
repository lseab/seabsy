#include <cctype>

#include "tokenization.hpp"


std::optional<int> bin_prec(TokenType type) {
    switch (type) {
        case TokenType::plus:
        case TokenType::minus:
            return 0;
        case TokenType::star:
        case TokenType::fslash:
            return 1;
        default:
            return {};
    }
}

Tokenizer::Tokenizer(const std::string& src)
    : m_src(src)
{
}

Token Tokenizer::create_token(TokenType type, std::optional<std::string> value) {
    return Token{type, .line_no = line_count, .value = value};
}

std::vector<Token> Tokenizer::tokenize() {
    std::vector<Token> tokens;
    std::string buffer;

    while (inspect().has_value()) {
        if (std::isalpha(inspect().value())) {
            buffer.push_back(consume());
            while (inspect().has_value() && std::isalnum(inspect().value())) {
                buffer.push_back(consume());
            }
            if (buffer == "return") {
                tokens.push_back(create_token(TokenType::_return));
                buffer.clear();
                continue;
            }
            if (buffer == "let") {
                tokens.push_back(create_token(TokenType::let));
                buffer.clear();
                continue;
            }
            if (buffer == "if") {
                tokens.push_back(create_token(TokenType::_if));
                buffer.clear();
                continue;
            }
            if (buffer == "elif") {
                tokens.push_back(create_token(TokenType::_elif));
                buffer.clear();
                continue;
            }
            if (buffer == "else") {
                tokens.push_back(create_token(TokenType::_else));
                buffer.clear();
                continue;
            }
            if (buffer == "exit") {
                tokens.push_back(create_token(TokenType::_exit));
                buffer.clear();
                continue;
            }
            tokens.push_back(create_token(TokenType::ident, buffer));
            buffer.clear();
            continue;
        }
        else if (std::isdigit(inspect().value())) {
            buffer.push_back(consume());
            while (inspect().has_value() && std::isdigit(inspect().value())) {
                buffer.push_back(consume());
            }
            tokens.push_back(create_token(TokenType::int_lit, buffer));
            buffer.clear();
        }
        else if (inspect().value() == ';') {
            consume();
            tokens.push_back(create_token(TokenType::semi));
        }
        else if (inspect().value() == '=') {
            consume();
            tokens.push_back(create_token(TokenType::eq));
        }
        else if (inspect().value() == '+') {
            consume();
            tokens.push_back(create_token(TokenType::plus));
        }
        else if (inspect().value() == '*') {
            consume();
            tokens.push_back(create_token(TokenType::star));
        }
        else if (inspect().value() == '-') {
            consume();
            tokens.push_back(create_token(TokenType::minus));
        }
        else if (inspect().value() == '/') {
            consume();
            if (inspect().has_value() && inspect().value() == '/') {
                while (inspect().has_value()) {
                    if (inspect().value() == '\n') {
                        consume();
                        break;
                    }
                    consume();
                }
            }
            else if (inspect().has_value() && inspect().value() == '*') {
                while (inspect().has_value()) {
                    if (inspect().value() == '*' && inspect(1).value() == '/') {
                        consume();
                        consume();
                        break;
                    }
                    consume();
                }
            }
            else {
                tokens.push_back(create_token(TokenType::fslash));
            }
        }
        else if (inspect().value() == '(') {
            consume();
            tokens.push_back(create_token(TokenType::left_paren));
        }
        else if (inspect().value() == ')') {
            consume();
            tokens.push_back(create_token(TokenType::right_paren));
        }
        else if (inspect().value() == '{') {
            consume();
            tokens.push_back(create_token(TokenType::open_curly));
        }
        else if (inspect().value() == '}') {
            consume();
            tokens.push_back(create_token(TokenType::close_curly));
        }
        else if (inspect().value() == '\n') {
            consume();
            line_count++;
        }
        else if (std::isspace(inspect().value())) {
            consume();
        }
    }

    m_index = 0;
    return tokens;
}

std::optional<char> Tokenizer::inspect(int offset) const {
    if (m_index + offset >= m_src.length()) return {};
    return m_src.at(m_index + offset);
}

char Tokenizer::consume() {
    char value = m_src.at(m_index);
    m_index++;
    return value;
}
