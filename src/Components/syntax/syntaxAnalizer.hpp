#pragma once

#include <cassert>
#include <vector>
#include <iostream>
#include <optional>
#include <variant>
#include "Components/lexing/lexicalAnalizer.hpp"
#include "Components/memory/memoryAllocator.hpp"
struct NodeExpr;
struct NodeTermIntLit
{
    Token int_lit;
};

struct NodeTermIdent
{
    Token ident;
};

struct NodeTermParen
{
    NodeExpr *expr;
};

struct NodeBinExprAdd
{
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExprMulti
{
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExprSub
{
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExprDiv
{
    NodeExpr *lhs;
    NodeExpr *rhs;
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
    NodeExpr *expr;
};

struct NodeStmtLet
{
    Token ident;
    NodeExpr *expr;
};

struct NodeStmt
{
    std::variant<NodeStmtExit *, NodeStmtLet *> var;
};

struct NodeProg
{
    std::vector<NodeStmt *> stmts;
};

class SyntaxAnalyzer
{

private:
    const std::vector<Token> m_tokens;
    size_t m_index = 0;
    MemoryAllocator m_allocator;
    [[nodiscard]] std::optional<Token> peek(int offset = 0) const;
    Token consume();
    Token try_consume(TokenType type, const std::string &err_msg);
    std::optional<Token> try_consume(TokenType type);

public:
    explicit SyntaxAnalyzer(std::vector<Token> tokens);
    std::optional<NodeTerm *> parse_term();
    std::optional<NodeExpr *> parse_expr(int min_prec = 0);
    std::optional<NodeStmt *> parse_stmt();
    std::optional<NodeProg> parse_prog();
};
