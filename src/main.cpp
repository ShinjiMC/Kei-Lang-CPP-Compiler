#include "Components/generator/generatorCode.hpp"
#include <cctype>
#include <cstring>
#include <fstream>
#include <iostream>

void show_usage(const char* program_name)
{
    std::cerr << "Incorrect usage. Correct usage is:" << std::endl;
    std::cerr << program_name << " <input.kei>" << std::endl;
}

bool has_correct_extension(const char* filename)
{
    const char* extension = ".kei";
    size_t len = std::strlen(filename);
    size_t ext_len = std::strlen(extension);
    return len >= ext_len && std::strcmp(filename + len - ext_len, extension) == 0;
}

void generateAndSaveOutput(const ProgramNode& prog)
{
    Generator generator(prog);
    std::fstream file("out.asm", std::ios::out);
    file << generator.gen_prog();
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        show_usage(argv[0]);
        return EXIT_FAILURE;
    }
    const char* filename = argv[1];
    if (!has_correct_extension(filename)) {
        std::cerr << "The input file must have a '.kei' extension." << std::endl;
        return EXIT_FAILURE;
    }
    std::ifstream input(filename);
    if (!input) {
        std::cerr << "Failed to open input file." << std::endl;
        return EXIT_FAILURE;
    }
    std::string contents((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    LexicalAnalyzer lexicalAnalyzer(std::move(contents));
    std::vector<Token> tokens = lexicalAnalyzer.tokenize();
    SyntaxAnalyzer syntaxAnalyzer(std::move(tokens));
    std::optional<ProgramNode> prog = syntaxAnalyzer.parseProgram();
    if (!prog.has_value()) {
        std::cerr << "Invalid program" << std::endl;
        exit(EXIT_FAILURE);
    }
    generateAndSaveOutput(prog.value());
    system("nasm -felf64 out.asm");
    system("ld -o out out.o");

    return EXIT_SUCCESS;
}