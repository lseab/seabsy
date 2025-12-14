#include <iostream>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>


enum class TokenType {
    _return,
    int_lit,
    semi,
};


struct Token {
    TokenType type;
    std::optional<std::string> value {};
};


std::vector<Token> tokenize(const std::string& str) {
    std::vector<Token> tokens;
    std::string buffer;
    for (int i = 0; i < str.length(); i++) {
        char c = str.at(i);
        if (std::isalpha(c)) {
            buffer.push_back(c);
            i++;
            while (std::isalnum((str.at(i)))) {
                buffer.push_back(str.at(i));
                i++;
            }
            i--;

            if (buffer == "return") {
                tokens.push_back(Token{.type = TokenType::_return});
                buffer.clear();
                continue;
            }
            else {
                std::cerr << "Unknown identifier: " << buffer << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else if (std::isdigit(c)) {
            buffer.push_back(c);
            i++;
            while (std::isdigit(str.at(i))) {
                buffer.push_back(str.at(i));
                i++;
            }
            i--;
            tokens.push_back(Token{.type = TokenType::int_lit, .value = buffer});
            buffer.clear();
        }
        else if (c == ';') {
            tokens.push_back(Token{.type = TokenType::semi});
        }
        else if (std::isspace(c)) {
            continue;
        }
    }
    return tokens;
}


std::string handle_int64_immediates(uint64_t immediate) {
    std::stringstream output;
    bool started = false;

    auto emit_hex16 = [&output](uint16_t chunk) {
        // Always print exactly 4 hex digits for a 16-bit chunk.
        output << "0x"
               << std::hex
               << std::setw(4) << std::setfill('0')
               << chunk
               << std::dec;
    };

    auto add_shift = [&](int shift) {
        output << ", lsl #" << shift;
    };

    for (int i = 0; i < 4; i++) {
        uint16_t chunk = static_cast<uint16_t>((immediate >> (i * 16)) & 0xFFFF);
        int shift = i * 16;
        if (!started) {
            if (chunk != 0) {
                output << "    movz x0, #";
                emit_hex16(chunk);
                if (shift != 0) add_shift(shift);
                output << "\n";
                started = true;
            }
        }
        else if (chunk != 0) {
            output << "    movk x0, #";
            emit_hex16(chunk);
            add_shift(shift);
            output << "\n";
        }
    }

    // If the immediate is 0, emit a single movz x0, #0
    if (!started) {
        output << "    movz x0, #";
        emit_hex16(0);
        output << "\n";
    }

    return output.str();
}


std::string tokens_to_asm(const std::vector<Token>& tokens) {
    std::stringstream output;
    output << ".globl _main\n.p2align 2\n_main:\n";
    for (int i = 0; i < tokens.size(); i++) {
        const Token& token = tokens.at(i);
        if (token.type == TokenType::_return) {
            if (
                i + 2 < tokens.size() &&
                tokens.at(i + 1).type == TokenType::int_lit &&
                tokens.at(i + 2).type == TokenType::semi
            ) {
                std::string int_string = tokens.at(i + 1).value.value();
                uint64_t int_value = std::stoll(int_string);
                std::string immediate_output = handle_int64_immediates(int_value);
                output << immediate_output;
                output << "    ret\n";
            }
        }
    }
    return output.str();
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Incorrect usage." << std::endl;
        std::cerr << "Correct usage: seabsy <file_name>.sy" << std::endl;
        return EXIT_FAILURE;
    }

    char* file_name = argv[1];
    std::ifstream file;
    file.open(file_name);
    if (file.fail()){
        std::cerr << "Couldn't open file" << std::endl;
        return EXIT_FAILURE;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string file_contents;
    file_contents = buffer.str();
    file.close();

    std::vector<Token> tokens = tokenize(file_contents);

    std::ofstream outfile ("test_files/out.asm");
    outfile << tokens_to_asm(tokens);
    outfile.close();

    return EXIT_SUCCESS;
}

