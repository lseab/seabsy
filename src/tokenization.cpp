#include <cctype>
#include <unordered_map>

#include "tokenization.hpp"


static const std::unordered_map<std::string, TokenType> keywords = {
    {"return", TokenType::_return},
    {"let", TokenType::let},
    {"if", TokenType::_if},
    {"elif", TokenType::_elif},
    {"else", TokenType::_else},
    {"exit", TokenType::_exit}
};

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

void Tokenizer::addToken(TokenType type, std::optional<std::string> value) {
    tokens.push_back(Token{type, .line_no = line_count, .value = value});
}

std::vector<Token> Tokenizer::tokenize() {
    tokens.clear();
    line_count = 1;
    m_index = 0;
    std::string buffer;

    while (inspect().has_value()) {
        char c = consume();
        switch (c) {
            case(';'):
                addToken(TokenType::semi);
                break;
            case('='):
                addToken(TokenType::eq);
                break;
            case('+'):
                addToken(TokenType::plus);
                break;
            case('*'):
                addToken(TokenType::star);
                break;
            case('-'):
                addToken(TokenType::minus);
                break;
            case('('):
                addToken(TokenType::left_paren);
                break;
            case(')'):
                addToken(TokenType::right_paren);
                break;
            case('{'):
                addToken(TokenType::open_curly);
                break;
            case('}'):
                addToken(TokenType::close_curly);
                break;
            case('/'):
                if (inspect().has_value() && inspect().value() == '/') {
                    while (inspect().has_value()) {
                        if (inspect().value() == '\n') {
                            consume();
                            line_count++;
                            break;
                        }
                        consume();
                    }
                }
                else if (inspect().has_value() && inspect().value() == '*') {
                    while (inspect().has_value()) {
                        if (inspect().value() == '*' && inspect(1).has_value() && inspect(1).value() == '/') {
                            consume();
                            consume();
                            break;
                        }
                        char next = consume();
                        if (next == '\n') {
                            line_count++;
                        }
                    }
                }
                else {
                    addToken(TokenType::fslash);
                }
                break;
            case('\n'):
                line_count++;
                break;
            default:
                if (std::isalpha(static_cast<unsigned char>(c))) {
                    buffer.push_back(c);
                    while (inspect().has_value() && std::isalnum(static_cast<unsigned char>(inspect().value()))) {
                        buffer.push_back(consume());
                    }
                    if (auto it = keywords.find(buffer); it != keywords.end()) {
                        addToken(it->second);
                        buffer.clear();
                    }
                    else {
                        addToken(TokenType::ident, buffer);
                        buffer.clear();
                    }
                }
                else if (std::isdigit(static_cast<unsigned char>(c))) {
                    buffer.push_back(c);
                    while (inspect().has_value() && std::isdigit(static_cast<unsigned char>(inspect().value()))) {
                        buffer.push_back(consume());
                    }
                    addToken(TokenType::int_lit, buffer);
                    buffer.clear();
                }
                break;
        }
    }

    m_index = 0;
    return Tokenizer::tokens;
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
