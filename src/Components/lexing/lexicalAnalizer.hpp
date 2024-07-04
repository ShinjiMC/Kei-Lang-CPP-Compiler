#pragma once

#include <string>
#include <vector>
#include <optional>
#include <iostream>

enum class TokenType
{
    exit,
    int_lit,
    semi,
    open_paren,
    close_paren,
    ident,
    let,
    eq,
    plus,
    star,
    sub,
    div
};

std::optional<int> bin_prec(TokenType type);

struct Token
{
    TokenType type;
    std::optional<std::string> value{};
};

class LexicalAnalizer
{
private:
    [[nodiscard]] std::optional<char> peek(int offset = 0) const;
    char consume();

    const std::string m_src;
    size_t m_index = 0;

public:
    explicit LexicalAnalizer(std::string src);
    std::vector<Token> tokenize();
};
