#pragma once

#include <array>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <vector>

enum class TokenType {
    exit,
    int_lit,
    semi,
    open_paren,
    close_pared,
    ident,
    let,
    eq,
    plus,
    star,
    minus,
    fslash,
    open_curly,
    close_curly,
    if_,
    elif,
    else_,
};

struct Token {
    TokenType type;
    int line;
    std::optional<std::string> value;
};

class LexicalAnalyzer {
private:
    std::string sourceCode;
    size_t currentIndex = 0;

    [[nodiscard]] char peek(size_t offset = 0) const;
    char consume();
    void skipWhitespace(int& lineCount);
    std::optional<Token> parseIdentifierOrKeyword(int lineCount);
    std::optional<Token> parseNumber(int lineCount);
    std::optional<Token> parseCommentOrSlash(int lineCount);
    std::optional<Token> parseSpecialCharacter(char c, int lineCount);

public:
    explicit LexicalAnalyzer(std::string src);
    std::vector<Token> tokenize();
};
const char* toString(TokenType type);
bool binaryPrecedence(TokenType type, int& precedence);