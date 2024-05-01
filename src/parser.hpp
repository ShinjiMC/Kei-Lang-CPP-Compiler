#pragma once

#include <vector>
#include <memory>
#include "tokenization.hpp"

class Generator;
struct NodeExpr
{
    virtual ~NodeExpr() = default;
    virtual void accept(Generator &generator) const = 0;
};

struct NodeExprIntLit : public NodeExpr
{
    Token int_lit;
    void accept(Generator &generator) const override;
};

struct NodeExprIdent : public NodeExpr
{
    Token ident;
    void accept(Generator &generator) const override;
};

struct NodeStmt
{
    virtual ~NodeStmt() = default;
    virtual void accept(Generator &generator) const = 0;
};

struct NodeStmtExit : public NodeStmt
{
    std::unique_ptr<NodeExpr> expr;
    void accept(Generator &generator) const override;
};

struct NodeStmtLet : public NodeStmt
{
    Token ident;
    std::unique_ptr<NodeExpr> expr;
    void accept(Generator &generator) const override;
};

struct NodeProg
{
    std::vector<std::unique_ptr<NodeStmt>> stmts;
};

class Parser
{
private:
    [[nodiscard]] inline Token peek(int offset = 0) const
    {
        if (m_index + offset >= m_numTokens)
        {
            return Token{.type = TokenType::inv, .value = ""};
        }
        else
        {
            return m_tokens[m_index + offset];
        }
    }

    inline Token consume()
    {
        return m_tokens[m_index++];
    }
    Token *m_tokens;
    size_t m_numTokens;
    size_t m_index = 0;

public:
    inline explicit Parser(Token *tokens, int numTokens)
        : m_tokens(tokens), m_numTokens(numTokens)
    {
    }

    bool parse_expr(std::unique_ptr<NodeExpr> &output)
    {
        if (peek().type != TokenType::inv && peek().type == TokenType::int_lit)
        {
            auto int_lit_expr = std::make_unique<NodeExprIntLit>();
            int_lit_expr->int_lit = consume();
            output = std::move(int_lit_expr);
            return true;
        }
        else if (peek().type != TokenType::inv && peek().type == TokenType::ident)
        {
            auto ident_expr = std::make_unique<NodeExprIdent>();
            ident_expr->ident = consume();
            output = std::move(ident_expr);
            return true;
        }
        else
        {
            return false;
        }
    }

    bool parse_stmt(std::unique_ptr<NodeStmt> &output)
    {
        if (peek().type == TokenType::exit && peek(1).type != TokenType::inv && peek(1).type == TokenType::open_paren)
        {
            consume();
            consume();
            std::unique_ptr<NodeStmtExit> stmt_exit = std::make_unique<NodeStmtExit>();
            std::unique_ptr<NodeExpr> node_expr_ptr;
            if (parse_expr(node_expr_ptr))
            {
                stmt_exit->expr = std::move(node_expr_ptr);
            }
            else
            {
                std::cerr << "Invalid expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            if (peek().type != TokenType::inv && peek().type == TokenType::close_paren)
            {
                consume();
            }
            else
            {
                std::cerr << "Expected `)`" << std::endl;
                exit(EXIT_FAILURE);
            }
            if (peek().type != TokenType::inv && peek().type == TokenType::semi)
            {
                consume();
            }
            else
            {
                std::cerr << "Expected `;`" << std::endl;
                exit(EXIT_FAILURE);
            }
            output = std::move(stmt_exit);
            return true;
        }
        else if (
            peek().type != TokenType::inv && peek().type == TokenType::let && peek(1).type != TokenType::inv && peek(1).type == TokenType::ident && peek(2).type != TokenType::inv && peek(2).type == TokenType::eq)
        {
            consume();
            std::unique_ptr<NodeStmtLet> stmt_let = std::make_unique<NodeStmtLet>();
            stmt_let->ident = consume();
            consume();
            std::unique_ptr<NodeExpr> node_expr_ptr;
            if (parse_expr(node_expr_ptr))
            {
                stmt_let->expr = std::move(node_expr_ptr);
            }
            else
            {
                std::cerr << "Invalid expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            if (peek().type != TokenType::inv && peek().type == TokenType::semi)
            {
                consume();
            }
            else
            {
                std::cerr << "Expected `;`" << std::endl;
                exit(EXIT_FAILURE);
            }
            output = std::move(stmt_let);
            return true;
        }
        else
        {
            return false;
        }
    }

    NodeProg parse_prog()
    {
        NodeProg prog;
        while (peek().type != TokenType::inv && m_index < m_numTokens)
        {
            // NodeStmt stmtNode;
            std::unique_ptr<NodeStmt> stmtNode;
            if (parse_stmt(stmtNode))
            {
                prog.stmts.push_back(std::move(stmtNode));
            }
            else
            {
                std::cerr << "Invalid statement" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        return prog;
    }
};