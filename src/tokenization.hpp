#pragma once

#include <iostream>
#include <cctype>
#include <cstring>

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
    inv
};

struct Token
{
    TokenType type;
    std::string value;
};

class Tokenizer
{
private:
    const char *m_src;
    size_t m_len;
    size_t m_index = 0;
    [[nodiscard]] inline char peak(int offset = 0) const
    {
        if (m_index + offset >= m_len)
        {
            return '\0';
        }
        return m_src[m_index + offset];
    }
    inline char consume()
    {
        return m_src[m_index++];
    }

public:
    static const int MAX_TOKENS = 1000;
    inline explicit Tokenizer(const char *src)
        : m_src(src), m_len(strlen(src))
    {
    }
    inline int tokenize(Token *tokens)
    {
        int tokenCount = 0;
        std::string buf;
        char currentChar;
        while ((currentChar = peak()) != '\0')
        {
            if (isalpha(currentChar))
            {
                do
                {
                    buf.push_back(consume());
                } while (isalnum(peak()));

                if (buf == "exit")
                {
                    tokens[tokenCount++] = {TokenType::exit, ""};
                    if (tokenCount >= MAX_TOKENS)
                        break;
                    buf.clear();
                    continue;
                }
                else if (buf == "let")
                {
                    tokens[tokenCount++] = {TokenType::let, ""};
                    if (tokenCount >= MAX_TOKENS)
                        break;
                    buf.clear();
                    continue;
                }
                else
                {
                    tokens[tokenCount++] = {TokenType::ident, buf};
                    if (tokenCount >= MAX_TOKENS)
                        break;
                    buf.clear();
                    continue;
                }
            }
            else if (isdigit(currentChar))
            {
                do
                {
                    buf.push_back(consume());
                } while (isdigit(peak()));
                tokens[tokenCount++] = {TokenType::int_lit, buf};
                if (tokenCount >= MAX_TOKENS)
                    break;
                buf.clear();
                continue;
            }
            else if (currentChar == '(')
            {
                consume();
                tokens[tokenCount++] = {TokenType::open_paren, ""};
                if (tokenCount >= MAX_TOKENS)
                    break;
                continue;
            }
            else if (currentChar == ')')
            {
                consume();
                tokens[tokenCount++] = {TokenType::close_paren, ""};
                if (tokenCount >= MAX_TOKENS)
                    break;
                continue;
            }
            else if (currentChar == '=')
            {
                consume();
                tokens[tokenCount++] = {TokenType::eq, ""};
                if (tokenCount >= MAX_TOKENS)
                    break;
                continue;
            }
            else if (currentChar == ';')
            {
                consume();
                tokens[tokenCount++] = {TokenType::semi, ""};
                if (tokenCount >= MAX_TOKENS)
                    break;
                continue;
            }
            else if (isspace(currentChar))
            {
                consume();
                continue;
            }
            else
            {
                std::cerr << "Invalid character encountered: " << currentChar << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        m_index = 0;
        return tokenCount;
    }
};