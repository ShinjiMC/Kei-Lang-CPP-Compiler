#pragma once

#include <cassert>
#include <unordered_map>
#include <sstream>
#include "Components/syntax/syntaxAnalizer.hpp"

class Generator
{
private:
    void push(const std::string &reg);
    void pop(const std::string &reg);
    struct Var
    {
        size_t stack_loc;
    };
    const NodeProg m_prog;
    std::stringstream m_output;
    size_t m_stack_size = 0;
    std::unordered_map<std::string, Var> m_vars;

public:
    explicit Generator(NodeProg prog);
    void gen_term(const NodeTerm *term);
    void gen_bin_expr(const NodeBinExpr *bin_expr);
    void gen_expr(const NodeExpr *expr);
    void gen_stmt(const NodeStmt *stmt);
    [[nodiscard]] std::string gen_prog();
};