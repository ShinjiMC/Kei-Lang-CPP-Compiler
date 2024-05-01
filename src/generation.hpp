#pragma once

#include "parser.hpp"
#include <unordered_map>

class Generator
{
private:
    void push(const char *reg)
    {
        m_output += "    push ";
        m_output += reg;
        m_output += "\n";
        m_stack_size++;
    }

    void pop(const char *reg)
    {
        m_output += "    pop ";
        m_output += reg;
        m_output += "\n";
        m_stack_size--;
    }

    struct Var
    {
        size_t stack_loc;
    };

    const NodeProg m_prog;
    std::string m_output;
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
        m_output += "    mov rax, ";
        m_output += expr_int_lit.int_lit.value;
        m_output += "\n";
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
        char buffer[128];
        int len = snprintf(buffer, sizeof(buffer), "QWORD [rsp + %zu]\n", (m_stack_size - var.stack_loc - 1) * 8);
        if (len < 0)
        {
            std::cerr << "Error formatting string.\n";
            exit(EXIT_FAILURE);
        }
        if (static_cast<size_t>(len) >= sizeof(buffer))
        {
            size_t needed_size = len + 1;
            char *dynamic_buffer = new char[needed_size];
            len = snprintf(dynamic_buffer, needed_size, "QWORD [rsp + %zu]\n", (m_stack_size - var.stack_loc - 1) * 8);
            if (len < 0 || static_cast<size_t>(len) >= needed_size)
            {
                std::cerr << "Error formatting string even with dynamic buffer.\n";
                delete[] dynamic_buffer;
                exit(EXIT_FAILURE);
            }
            push(dynamic_buffer);
            delete[] dynamic_buffer;
        }
        else
            push(buffer);
    }

    void gen_stmt(const NodeStmt &stmt)
    {
        stmt.accept(*this);
    }

    void gen_stmtExit(const NodeStmtExit &stmt_exit)
    {
        gen_expr(stmt_exit.expr);
        m_output += "    mov rax, 60\n";
        pop("rdi");
        m_output += "    syscall\n";
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
        m_output += "global _start\n_start:\n";
        for (const auto &stmt : m_prog.stmts)
            gen_stmt(*stmt);
        m_output += "    mov rax, 60\n";
        m_output += "    mov rdi, 0\n";
        m_output += "    syscall\n";
        return m_output;
    }
};