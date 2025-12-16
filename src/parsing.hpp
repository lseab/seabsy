#include "tokenization.hpp"


struct NodeExpr {
    Token int_lit;
};


struct NodeReturn {
    NodeExpr expr;
};


class Parser {
public:
    inline Parser(const std::vector<Token> tokens):
        m_tokens(tokens)
        {
        };

    inline std::optional<NodeExpr> parse_expr() {
        if (inspect().has_value() && inspect().value().type == TokenType::int_lit) {
            return NodeExpr{.int_lit = inspect().value()};
        }
        return {};
    };

    inline std::optional<NodeReturn> parse() {
        std::optional<NodeReturn> exit_node;
        while (inspect().has_value()) {
            if (inspect().value().type == TokenType::_return) {
                consume();
                if (auto node_expr = parse_expr()) {
                    exit_node = NodeReturn{.expr = node_expr.value()};
                    consume();
                }
                else {
                    std::cerr << "Inalid expression" << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
            if (inspect().value().type == TokenType::semi) {
                consume();
            }
        }
        return exit_node;
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
};