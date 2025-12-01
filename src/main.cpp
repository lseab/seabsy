#include <iostream>
#include <fstream>
#include <optional>
#include <sstream>
#include <vector>


enum class TokenType {
    _return,
    int_lit,
    semi,
};


struct Token {
    TokenType type;
    std::optional<std::string> value;
};


std::vector<Token> tokenize(const std::string& contents) {
    for (char c: contents) {
        std::cout << c << std::endl;
    }
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

    tokenize(file_contents);

    return EXIT_SUCCESS;
}

