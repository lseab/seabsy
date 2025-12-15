#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "tokenization.hpp"
#include "generator.hpp"


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

    Tokenizer tokenizer(file_contents);
    std::vector<Token> tokens = tokenizer.tokenize();
    Parser parser(tokens);
    std::optional<NodeReturn> tree = parser.parse();
    std::optional<Generator> generator(tree);

    std::ofstream outfile ("test_files/out.asm");
    outfile << generator->generate();
    outfile.close();

    return EXIT_SUCCESS;
}

