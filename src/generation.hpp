#pragma once

#include "parser.hpp"
#include <unordered_map>
#include <variant>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <vector>
#include <variant>
class Generator
{
private:
    void push(const std::string &reg)
    {
        m_output << "    push " << reg << "\n";
        m_stack_size++;
    }

    void pop(const std::string &reg)
    {
        m_output << "    pop " << reg << "\n";
        m_stack_size--;
    }

    struct Var
    {
        size_t stack_loc;
    };

    const NodeProg m_prog;
    std::stringstream m_output;
    size_t m_stack_size = 0;
    std::unordered_map<std::string, Var> m_vars{};

public:
    inline explicit Generator(NodeProg &&prog)
        : m_prog(std::move(prog))
    {
    }

    void gen_expr(const std::unique_ptr<NodeExpr> &expr)
    {
        expr->accept(*this);
    }

    void gen_exprIntLit(const NodeExprIntLit &expr_int_lit)
    {
        m_output << "    mov rax, " << expr_int_lit.int_lit.value << "\n";
        push("rax");
    }
    void gen_exprIdent(const NodeExprIdent &expr_ident)
    {
        if (!m_vars.count(expr_ident.ident.value))
        {
            std::cerr << "Undeclared identifier: " << expr_ident.ident.value << std::endl;
            exit(EXIT_FAILURE);
        }
        const auto &var = m_vars.at(expr_ident.ident.value);
        std::stringstream offset;
        offset << "QWORD [rsp + " << (m_stack_size - var.stack_loc - 1) * 8 << "]\n";
        push(offset.str());
    }

    void gen_stmt(const NodeStmt &stmt)
    {
        stmt.accept(*this);
    }

    void gen_stmtExit(const NodeStmtExit &stmt_exit)
    {
        gen_expr(stmt_exit.expr);
        m_output << "    mov rax, 60\n";
        pop("rdi");
        m_output << "    syscall\n";
    }

    void gen_stmtLet(const NodeStmtLet &stmt_let)
    {
        if (m_vars.count(stmt_let.ident.value))
        {
            std::cerr << "Identifier already used: " << stmt_let.ident.value << std::endl;
            exit(EXIT_FAILURE);
        }
        m_vars.insert({stmt_let.ident.value, Var{.stack_loc = m_stack_size}});
        gen_expr(stmt_let.expr);
    }

    [[nodiscard]] std::string gen_prog()
    {
        m_output << "global _start\n_start:\n";

        for (const auto &stmt : m_prog.stmts)
        {
            gen_stmt(*stmt);
        }

        m_output << "    mov rax, 60\n";
        m_output << "    mov rdi, 0\n";
        m_output << "    syscall\n";
        return m_output.str();
    }
};