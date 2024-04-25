#pragma once

#include <cassert>
#include <variant>

#include "arena.hpp"
#include "tokenization.hpp"

struct NodeTermIntLit
{
    Token int_lit;
};

struct NodeTermIdent
{
    Token ident;
};

struct NodeExpr;

struct NodeTermParen
{
    std::unique_ptr<NodeExpr> expr;
};

struct NodeBinExprAdd
{
    std::unique_ptr<NodeExpr> lhs;
    std::unique_ptr<NodeExpr> rhs;
};

struct NodeBinExprMulti
{
    std::unique_ptr<NodeExpr> lhs;
    std::unique_ptr<NodeExpr> rhs;
};

struct NodeBinExprSub
{
    std::unique_ptr<NodeExpr> lhs;
    std::unique_ptr<NodeExpr> rhs;
};

struct NodeBinExprDiv
{
    std::unique_ptr<NodeExpr> lhs;
    std::unique_ptr<NodeExpr> rhs;
};

struct NodeBinExpr
{
    std::variant<NodeBinExprAdd *, NodeBinExprMulti *, NodeBinExprSub *, NodeBinExprDiv *> var;
};

struct NodeTerm
{
    std::variant<NodeTermIntLit *, NodeTermIdent *, NodeTermParen *> var;
};

struct NodeExpr
{
    std::variant<NodeTerm *, NodeBinExpr *> var;
};

struct NodeStmtExit
{
    std::unique_ptr<NodeExpr> expr;
};

struct NodeStmtLet
{
    Token ident;
    std::unique_ptr<NodeExpr> expr;
};

struct NodeStmt;

struct NodeScope
{
    std::vector<std::unique_ptr<NodeStmt>> stmts;
};

struct NodeIfPred;

struct NodeIfPredElif
{
    std::unique_ptr<NodeExpr> expr;
    std::unique_ptr<NodeScope> scope;
    std::unique_ptr<NodeIfPred> pred;
};

struct NodeIfPredElse
{
    std::unique_ptr<NodeScope> scope;
};

struct NodeIfPred
{
    std::variant<NodeIfPredElif, NodeIfPredElse> var;
};

struct NodeStmtIf
{
    std::unique_ptr<NodeExpr> expr;
    std::unique_ptr<NodeScope> scope;
    std::unique_ptr<NodeIfPred> pred;
};

struct NodeStmtAssign
{
    Token ident;
    std::unique_ptr<NodeExpr> expr;
};

struct NodeStmt
{
    std::variant<NodeStmtExit *, NodeStmtLet *, NodeScope *, NodeStmtIf *, NodeStmtAssign *> var;
};

struct NodeProg
{
    std::vector<std::unique_ptr<NodeStmt>> stmts;
};

class Parser
{
private:
    const std::vector<Token> m_tokens;
    size_t m_index = 0;
    ArenaAllocator m_allocator{1024 * 1024 * 4}; // 4 mb

    [[nodiscard]] Token peek(const int offset = 0) const
    {
        if (m_index + offset >= m_tokens.size())
        {
            return {TokenType::eof, 0, nullptr};
        }
        return m_tokens.at(m_index + offset);
    }

    Token consume()
    {
        return m_tokens.at(m_index++);
    }

    Token try_consume_err(const TokenType type)
    {
        if (peek().type == type)
        {
            return consume();
        }
        error_expected(to_string(type));
        return {TokenType::eof, 0, nullptr};
    }

    Token try_consume(const TokenType type)
    {
        if (peek().type == type)
        {
            return consume();
        }
        return {TokenType::unknown, 0, nullptr};
    }

public:
    explicit Parser(std::vector<Token> tokens)
        : m_tokens(std::move(tokens))
    {
    }

    void error_expected(const std::string &expected)
    {
        const auto token = peek();
        std::cerr << "Error: Expected " << expected << " at line " << token.line << ", got " << to_string(token.type) << std::endl;
        std::exit(EXIT_FAILURE);
    }

    NodeTerm *parse_term()
    {
        auto int_lit = try_consume(TokenType::int_lit);
        if (int_lit.type != TokenType::unknown)
        {
            auto term_int_lit = m_allocator.emplace<NodeTermIntLit>(int_lit);
            return m_allocator.emplace<NodeTerm>(term_int_lit);
        }
        auto ident = try_consume(TokenType::ident);
        if (ident.type != TokenType::unknown)
        {
            auto expr_ident = m_allocator.emplace<NodeTermIdent>(ident);
            return m_allocator.emplace<NodeTerm>(expr_ident);
        }
        if (try_consume(TokenType::open_paren).type != TokenType::unknown)
        {
            auto expr = parse_expr();
            if (!expr)
            {
                error_expected("expression");
            }
            try_consume_err(TokenType::close_paren);
            auto term_paren = m_allocator.emplace<NodeTermParen>(expr);
            return m_allocator.emplace<NodeTerm>(term_paren);
        }
        error_expected("term");
        return nullptr;
    }

    NodeExpr *parse_expr(const int min_prec = 0)
    {
        NodeTerm *term_lhs = parse_term();
        if (!term_lhs)
        {
            error_expected("term");
        }
        auto expr_lhs = m_allocator.emplace<NodeExpr>(term_lhs);
        while (true)
        {
            Token curr_tok = peek();
            if (curr_tok.type == TokenType::eof || curr_tok.type == TokenType::unknown)
            {
                break;
            }
            auto prec = bin_prec(curr_tok.type);
            if (!prec || prec < min_prec)
            {
                break;
            }
            const auto [type, line, value] = consume();
            NodeExpr *expr_rhs = parse_expr(prec + 1);
            if (!expr_rhs)
            {
                error_expected("expression");
            }
            auto expr = m_allocator.emplace<NodeBinExpr>();
            auto expr_lhs2 = m_allocator.emplace<NodeExpr>();
            switch (curr_tok.type)
            {
            case TokenType::plus:
                expr_lhs2->var = expr_lhs->var;
                expr->var = m_allocator.emplace<NodeBinExprAdd>(&expr_lhs2, expr_rhs);
                break;
            case TokenType::star:
                expr_lhs2->var = expr_lhs->var;
                expr->var = m_allocator.emplace<NodeBinExprMulti>(&expr_lhs2, expr_rhs);
                break;
            case TokenType::minus:
                expr_lhs2->var = expr_lhs->var;
                expr->var = m_allocator.emplace<NodeBinExprSub>(&expr_lhs2, expr_rhs);
                break;
            case TokenType::fslash:
                expr_lhs2->var = expr_lhs->var;
                expr->var = m_allocator.emplace<NodeBinExprDiv>(&expr_lhs2, expr_rhs);
                break;
            default:
                assert(false);
                break;
            }
            expr_lhs->var = expr;
        }
        return expr_lhs;
    }
};