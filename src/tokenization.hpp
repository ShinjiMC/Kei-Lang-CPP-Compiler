#pragma once

#include <string>
#include <vector>
#include <cassert>
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
    minus,
    fslash,
    open_curly,
    close_curly,
    if_,
    elif,
    else_,
    eof,
    unknown,
};

inline const char *to_string(TokenType type)
{
    static const char *token_strings[] = {
        "`exit`", "int literal", "`;`", "`(`", "`)`", "identifier",
        "`let`", "`=`", "`+`", "`*`", "`-`", "`/`", "`{`", "`}`",
        "`if`", "`elif`", "`else`"};
    assert(static_cast<int>(type) >= 0 && static_cast<int>(type) < sizeof(token_strings) / sizeof(token_strings[0]));
    return token_strings[static_cast<int>(type)];
}

inline int bin_prec(const TokenType type)
{
    switch (type)
    {
    case TokenType::minus:
    case TokenType::plus:
        return 0;
    case TokenType::fslash:
    case TokenType::star:
        return 1;
    default:
        return -1;
    }
}

struct Token
{
    TokenType type;
    int line;
    std::string_view value;

    Token(TokenType type, int line, std::string_view value)
        : type(type), line(line), value(value) {}
};

class Tokenizer
{
private:
    const std::string m_src;
    size_t m_index = 0;
    [[nodiscard]] char peek(const size_t offset = 0) const
    {
        if (m_index + offset >= m_src.length())
            return '\0';
        return m_src.at(m_index + offset);
    }

    char consume()
    {
        char result = m_src.at(m_index);
        ++m_index;
        return result;
    }

public:
    explicit Tokenizer(std::string src)
        : m_src(std::move(src))
    {
    }

    std::vector<Token> tokenize()
    {
        std::vector<Token> tokens;
        std::string buf;
        int line_count = 1;
        while (peek() != '\0')
        {
            if (std::isalpha(peek()))
            {
                buf.push_back(consume());
                while (peek() != '\0' && std::isalnum(peek()))
                {
                    buf.push_back(consume());
                }
                if (buf == "exit")
                {
                    tokens.push_back({TokenType::exit, line_count, nullptr});
                    buf.clear();
                }
                else if (buf == "let")
                {
                    tokens.push_back({TokenType::let, line_count, nullptr});
                    buf.clear();
                }
                else if (buf == "if")
                {
                    tokens.push_back({TokenType::if_, line_count, nullptr});
                    buf.clear();
                }
                else if (buf == "elif")
                {
                    tokens.push_back({TokenType::elif, line_count, nullptr});
                    buf.clear();
                }
                else if (buf == "else")
                {
                    tokens.push_back({TokenType::else_, line_count, nullptr});
                    buf.clear();
                }
                else
                {
                    tokens.push_back({TokenType::ident, line_count, buf});
                    buf.clear();
                }
            }
            else if (std::isdigit(peek()))
            {
                buf.push_back(consume());
                while (peek() != '\0' && std::isdigit(peek()))
                {
                    buf.push_back(consume());
                }
                tokens.push_back({TokenType::int_lit, line_count, buf});
                buf.clear();
            }
            else if (peek() == '/' && peek(1) != '\0' && peek(1) == '/')
            {
                consume();
                consume();
                while (peek() != '\0' && peek() != '\n')
                {
                    consume();
                }
            }
            else if (peek() == '/' && peek(1) != '\0' && peek(1) == '*')
            {
                consume();
                consume();
                while (peek() != '\0')
                {
                    if (peek() == '*' && peek(1) != '\0' && peek(1) == '/')
                    {
                        break;
                    }
                    consume();
                }
                if (peek() != '\0')
                {
                    consume();
                }
                if (peek() != '\0')
                {
                    consume();
                }
            }
            else if (peek() == '(')
            {
                consume();
                tokens.push_back({TokenType::open_paren, line_count, nullptr});
            }
            else if (peek() == ')')
            {
                consume();
                tokens.push_back({TokenType::close_paren, line_count, nullptr});
            }
            else if (peek() == ';')
            {
                consume();
                tokens.push_back({TokenType::semi, line_count, nullptr});
            }
            else if (peek() == '=')
            {
                consume();
                tokens.push_back({TokenType::eq, line_count, nullptr});
            }
            else if (peek() == '+')
            {
                consume();
                tokens.push_back({TokenType::plus, line_count, nullptr});
            }
            else if (peek() == '*')
            {
                consume();
                tokens.push_back({TokenType::star, line_count, nullptr});
            }
            else if (peek() == '-')
            {
                consume();
                tokens.push_back({TokenType::minus, line_count, nullptr});
            }
            else if (peek() == '/')
            {
                consume();
                tokens.push_back({TokenType::fslash, line_count, nullptr});
            }
            else if (peek() == '{')
            {
                consume();
                tokens.push_back({TokenType::open_curly, line_count, nullptr});
            }
            else if (peek() == '}')
            {
                consume();
                tokens.push_back({TokenType::close_curly, line_count, nullptr});
            }
            else if (peek() == '\n')
            {
                consume();
                line_count++;
            }
            else if (std::isspace(peek()))
            {
                consume();
            }
            else
            {
                // throw std::runtime_error("Unexpected character: " + std::string(1, peek()));
                std::cerr << "Invalid token" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        m_index = 0;
        return tokens;
    }
};