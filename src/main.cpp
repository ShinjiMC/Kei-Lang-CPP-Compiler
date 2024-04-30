#include <fstream>
#include <iostream>
#include <cctype>
#include <cstring>
#include "generation.hpp"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Incorrect usage. Correct usage is..." << std::endl;
        std::cerr << "kei_lang <input.kei>" << std::endl;
        return EXIT_FAILURE;
    }
    const char *filename = argv[1];
    if (strlen(filename) < 5 || strcmp(filename + strlen(filename) - 4, ".kei") != 0)
    {
        std::cerr << "The input file must have a '.kei' extension." << std::endl;
        return EXIT_FAILURE;
    }

    std::ifstream input(filename);
    if (!input)
    {
        std::cerr << "Failed to open input file." << std::endl;
        return EXIT_FAILURE;
    }
    std::string contents((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    Tokenizer tokenizer(contents.c_str());
    Token tokens[Tokenizer::MAX_TOKENS];
    int numTokens = tokenizer.tokenize(tokens);
    if (numTokens == 0)
    {
        std::cerr << "No tokens generated" << std::endl;
        return EXIT_FAILURE;
    }

    Parser parser(tokens, numTokens);
    NodeProg prog = parser.parse_prog();
    /*
    Generator generator(exitNode);
    {
        std::fstream file("out.asm", std::ios::out);
        file << generator.gen_prog();
    }
    std::vector<Token> tokens2;
    for (int i = 0; i < numTokens; i++)
    {
        tokens2.push_back(tokens[i]);
    }
    Parser parser(std::move(tokens2));
    std::optional<NodeProg> prog = parser.parse_prog();

    if (!prog.has_value())
    {
        std::cerr << "Invalid program" << std::endl;
        exit(EXIT_FAILURE);
    }*/
    Generator generator(std::move(prog));
    {
        std::fstream file("out.asm", std::ios::out);
        file << generator.gen_prog();
    }
    system("nasm -felf64 out.asm");
    system("ld -o out out.o");

    return EXIT_SUCCESS;
}