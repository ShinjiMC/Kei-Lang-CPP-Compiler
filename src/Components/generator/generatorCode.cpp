#include "generatorCode.hpp"

Generator::Generator(ProgramNode prog)
    : m_prog(std::move(prog))
{
}

void Generator::gen_term(const TermNode* term)
{
    struct TermVisitor {
        Generator& gen;
        void operator()(const IntLiteralNode* lit) const
        {
            gen.m_output << "    mov rax, " << lit->intLit.value.value() << "\n";
            gen.push("rax");
        }
        void operator()(const IdentifierNode* ident) const
        {
            const auto it = std::find_if(gen.m_vars.begin(), gen.m_vars.end(), [&](const Var& var) {
                return var.name == ident->ident.value.value();
            });
            if (it == gen.m_vars.cend()) {
                std::cerr << "Undeclared identifier: " << ident->ident.value.value() << std::endl;
                exit(EXIT_FAILURE);
            }
            std::stringstream offset;
            offset << "QWORD [rsp + " << (gen.m_stack_size - it->stack_loc - 1) * 8 << "]";
            gen.push(offset.str());
        }
        void operator()(const ParenthesizedExprNode* paren) const
        {
            gen.gen_expr(paren->expr);
        }
    };
    TermVisitor visitor({ .gen = *this });
    std::visit(visitor, term->term);
}

void Generator::gen_bin_expr(const BinaryExpressionNode* bin_expr)
{
    struct BinExprVisitor {
        Generator& gen;

        void operator()(const SubtractionNode* sub) const
        {
            gen.gen_expr(sub->rhs);
            gen.gen_expr(sub->lhs);
            gen.pop("rax");
            gen.pop("rbx");
            gen.m_output << "    sub rax, rbx\n";
            gen.push("rax");
        }

        void operator()(const AdditionNode* add) const
        {
            gen.gen_expr(add->rhs);
            gen.gen_expr(add->lhs);
            gen.pop("rax");
            gen.pop("rbx");
            gen.m_output << "    add rax, rbx\n";
            gen.push("rax");
        }

        void operator()(const MultiplicationNode* multi) const
        {
            gen.gen_expr(multi->rhs);
            gen.gen_expr(multi->lhs);
            gen.pop("rax");
            gen.pop("rbx");
            gen.m_output << "    mul rbx\n";
            gen.push("rax");
        }

        void operator()(const DivisionNode* div) const
        {
            gen.gen_expr(div->rhs);
            gen.gen_expr(div->lhs);
            gen.pop("rax");
            gen.pop("rbx");
            gen.m_output << "    div rbx\n";
            gen.push("rax");
        }
    };

    BinExprVisitor visitor { .gen = *this };
    std::visit(visitor, bin_expr->operation);
}

void Generator::gen_expr(const ExprNode* expr)
{
    struct ExprVisitor {
        Generator& gen;

        void operator()(const TermNode* term) const
        {
            gen.gen_term(term);
        }

        void operator()(const BinaryExpressionNode* bin_expr) const
        {
            gen.gen_bin_expr(bin_expr);
        }
    };

    ExprVisitor visitor { .gen = *this };
    std::visit(visitor, expr->expression);
}

void Generator::gen_scope(const ScopeNode* scope)
{
    begin_scope();
    for (const StmtNode* stmt : scope->statements) {
        gen_stmt(stmt);
    }
    end_scope();
}

void Generator::gen_if_pred(const ElseIfNode* pred, const std::string& end_label)
{
    struct PredVisitor {
        Generator& gen;
        const std::string& end_label;

        void operator()(const ElseIfBranchNode* elif) const
        {
            gen.m_output << "    ;; elif\n";
            gen.gen_expr(elif->condition);
            gen.pop("rax");
            const std::string label = gen.create_label();
            gen.m_output << "    test rax, rax\n";
            gen.m_output << "    jz " << label << "\n";
            gen.gen_scope(elif->scope);
            gen.m_output << "    jmp " << end_label << "\n";
            if (elif->next.has_value()) {
                gen.m_output << label << ":\n";
                gen.gen_if_pred(elif->next.value(), end_label);
            }
        }

        void operator()(const ElseBranchNode* else_) const
        {
            gen.m_output << "    ;; else\n";
            gen.gen_scope(else_->scope);
        }
    };

    PredVisitor visitor { .gen = *this, .end_label = end_label };
    std::visit(visitor, pred->branch);
}

void Generator::gen_stmt(const StmtNode* stmt)
{
    struct StmtVisitor {
        Generator& gen;

        void operator()(const ExitStatementNode* stmt_exit) const
        {
            gen.m_output << "    ;; exit\n";
            gen.gen_expr(stmt_exit->expr);
            gen.m_output << "    mov rax, 60\n";
            gen.pop("rdi");
            gen.m_output << "    syscall\n";
            gen.m_output << "    ;; /exit\n";
        }

        void operator()(const LetStatementNode* stmt_let) const
        {
            gen.m_output << "    ;; let\n";
            if (std::find_if(
                    gen.m_vars.begin(),
                    gen.m_vars.end(),
                    [&](const Var& var) { return var.name == stmt_let->ident.value.value(); })
                != gen.m_vars.end()) {
                std::cerr << "Identifier already used: " << stmt_let->ident.value.value() << std::endl;
                exit(EXIT_FAILURE);
            }
            gen.m_vars.push_back({ .name = stmt_let->ident.value.value(), .stack_loc = gen.m_stack_size });
            gen.gen_expr(stmt_let->expr);
            gen.m_output << "    ;; /let\n";
        }

        void operator()(const AssignmentStatementNode* stmt_assign) const
        {
            const auto it = std::find_if(gen.m_vars.begin(), gen.m_vars.end(), [&](const Var& var) {
                return var.name == stmt_assign->ident.value.value();
            });
            if (it == gen.m_vars.end()) {
                std::cerr << "Undeclared identifier: " << stmt_assign->ident.value.value() << std::endl;
                exit(EXIT_FAILURE);
            }
            gen.gen_expr(stmt_assign->expr);
            gen.pop("rax");
            gen.m_output << "    mov [rsp + " << (gen.m_stack_size - it->stack_loc - 1) * 8 << "], rax\n";
        }

        void operator()(const ScopeNode* scope) const
        {
            gen.m_output << "    ;; scope\n";
            gen.gen_scope(scope);
            gen.m_output << "    ;; /scope\n";
        }

        void operator()(const IfStatementNode* stmt_if) const
        {
            gen.m_output << "    ;; if\n";
            gen.gen_expr(stmt_if->condition);
            gen.pop("rax");
            const std::string label = gen.create_label();
            gen.m_output << "    test rax, rax\n";
            gen.m_output << "    jz " << label << "\n";
            gen.gen_scope(stmt_if->scope);
            if (stmt_if->pred.has_value()) {
                const std::string end_label = gen.create_label();
                gen.m_output << "    jmp " << end_label << "\n";
                gen.m_output << label << ":\n";
                gen.gen_if_pred(stmt_if->pred.value(), end_label);
                gen.m_output << end_label << ":\n";
            }
            else {
                gen.m_output << label << ":\n";
            }
            gen.m_output << "    ;; /if\n";
        }
    };

    StmtVisitor visitor { .gen = *this };
    std::visit(visitor, stmt->statement);
}

[[nodiscard]] std::string Generator::gen_prog()
{
    m_output << "global _start\n_start:\n";

    for (const StmtNode* stmt : m_prog.statements) {
        gen_stmt(stmt);
    }

    m_output << "    mov rax, 60\n";
    m_output << "    mov rdi, 0\n";
    m_output << "    syscall\n";
    return m_output.str();
}

void Generator::push(const std::string& reg)
{
    m_output << "    push " << reg << "\n";
    m_stack_size++;
}

void Generator::pop(const std::string& reg)
{
    m_output << "    pop " << reg << "\n";
    m_stack_size--;
}

void Generator::begin_scope()
{
    m_scopes.push_back(m_vars.size());
}

void Generator::end_scope()
{
    const size_t pop_count = m_vars.size() - m_scopes.back();
    if (pop_count != 0) {
        m_output << "    add rsp, " << pop_count * 8 << "\n";
    }
    m_stack_size -= pop_count;
    for (size_t i = 0; i < pop_count; i++) {
        m_vars.pop_back();
    }
    m_scopes.pop_back();
}

std::string Generator::create_label()
{
    std::stringstream ss;
    ss << "label" << m_label_count;
    m_label_count++;
    return ss.str();
}