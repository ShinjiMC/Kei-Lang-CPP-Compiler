#pragma once

#include "Components/syntax/syntaxAnalyzer.hpp"
#include <algorithm>
#include <ranges>
#include <sstream>

class Generator {
private:
    struct Var {
        std::string name;
        size_t stack_loc;
    };
    const ProgramNode m_prog;
    std::stringstream m_output;
    size_t m_stack_size = 0;
    std::vector<Var> m_vars {};
    std::vector<size_t> m_scopes {};
    int m_label_count = 0;
    void push(const std::string& reg);
    void pop(const std::string& reg);
    void begin_scope();
    void end_scope();
    std::string create_label();

public:
    explicit Generator(ProgramNode prog);
    void gen_term(const TermNode* term);
    void gen_bin_expr(const BinaryExpressionNode* bin_expr);
    void gen_expr(const ExprNode* expr);
    void gen_scope(const ScopeNode* scope);
    void gen_if_pred(const ElseIfNode* pred, const std::string& end_label);
    void gen_stmt(const StmtNode* stmt);
    [[nodiscard]] std::string gen_prog();
};