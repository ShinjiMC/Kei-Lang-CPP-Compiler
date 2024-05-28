#include "lexicalAnalyzer.hpp"
#include <cassert>
#include <cctype>

const char* toString(TokenType type)
{
    static const std::array<const char*, 17> tokenStrings
        = { "`exit`", "int literal", "`;`", "`(`", "`)`", "identifier", "`let`",  "`=`", "`+`",
            "`*`",    "`-`",         "`/`", "`{`", "`}`", "`if`",       "`elif`", "`7`" };
    assert(static_cast<int>(type) >= 0 && static_cast<int>(type) < tokenStrings.size());
    return tokenStrings[static_cast<int>(type)];
}

bool binaryPrecedence(const TokenType type, int& precedence)
{
    switch (type) {
    case TokenType::minus:
    case TokenType::plus:
        precedence = 0;
        return true;
    case TokenType::fslash:
    case TokenType::star:
        precedence = 1;
        return true;
    default:
        return false;
    }
}

LexicalAnalyzer::LexicalAnalyzer(std::string src)
    : sourceCode(std::move(src))
{
}

char LexicalAnalyzer::peek(size_t offset) const
{
    if (currentIndex + offset < sourceCode.length())
        return sourceCode.at(currentIndex + offset);
    return '\0';
}

char LexicalAnalyzer::consume()
{
    char result = sourceCode.at(currentIndex);
    currentIndex++;
    return result;
}

void LexicalAnalyzer::skipWhitespace(int& lineCount)
{
    while (std::isspace(peek())) {
        if (peek() == '\n')
            lineCount++;
        consume();
    }
}

std::optional<Token> LexicalAnalyzer::parseIdentifierOrKeyword(int lineCount)
{
    std::string buffer;
    while (std::isalnum(peek())) {
        buffer.push_back(consume());
    }
    static const std::map<std::string, TokenType, std::less<>> keywords
        = { { "exit", TokenType::exit },
            { "let", TokenType::let },
            { "if", TokenType::if_ },
            { "elif", TokenType::elif },
            { "else", TokenType::else_ } };
    if (auto it = keywords.find(buffer); it != keywords.end()) {
        return Token { it->second, lineCount };
    }
    return Token { TokenType::ident, lineCount, buffer };
}

std::optional<Token> LexicalAnalyzer::parseNumber(int lineCount)
{
    std::string buffer;
    while (std::isdigit(peek())) {
        buffer.push_back(consume());
    }
    return Token { TokenType::int_lit, lineCount, buffer };
}

std::optional<Token> LexicalAnalyzer::parseCommentOrSlash(int lineCount)
{
    if (peek(1) == '/') {
        consume();
        consume();
        while (peek() != '\n' && peek() != '\0') {
            consume();
        }
        return std::nullopt;
    }
    else if (peek(1) == '*') {
        consume();
        consume();
        while (!(peek() == '*' && peek(1) == '/') && peek() != '\0') {
            consume();
        }
        if (peek() != '\0')
            consume();
        if (peek() != '\0')
            consume();
        return std::nullopt;
    }
    else {
        consume();
        return Token { TokenType::fslash, lineCount };
    }
}

std::optional<Token> LexicalAnalyzer::parseSpecialCharacter(char c, int lineCount)
{
    const std::map<char, TokenType> specialTokens
        = { { '(', TokenType::open_paren }, { ')', TokenType::close_pared }, { ';', TokenType::semi },
            { '=', TokenType::eq },         { '+', TokenType::plus },        { '*', TokenType::star },
            { '-', TokenType::minus },      { '{', TokenType::open_curly },  { '}', TokenType::close_curly } };
    if (auto it = specialTokens.find(c); it != specialTokens.end()) {
        consume();
        return Token { it->second, lineCount };
    }
    else {
        std::cerr << "Invalid token: " << c << std::endl;
        exit(EXIT_FAILURE);
    }
}

std::vector<Token> LexicalAnalyzer::tokenize()
{
    std::vector<Token> tokens;
    int lineCount = 1;
    char c;
    while ((c = peek()) != '\0') {
        if (std::isspace(c)) {
            skipWhitespace(lineCount);
        }
        else if (std::isalpha(c)) {
            if (auto token = parseIdentifierOrKeyword(lineCount)) {
                tokens.push_back(*token);
            }
        }
        else if (std::isdigit(c)) {
            if (auto token = parseNumber(lineCount)) {
                tokens.push_back(*token);
            }
        }
        else if (c == '/') {
            if (auto token = parseCommentOrSlash(lineCount)) {
                tokens.push_back(*token);
            }
        }
        else {
            if (auto token = parseSpecialCharacter(c, lineCount)) {
                tokens.push_back(*token);
            }
        }
    }
    currentIndex = 0;
    return tokens;
}