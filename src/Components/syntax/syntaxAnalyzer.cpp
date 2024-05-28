#include "syntaxAnalyzer.hpp"

SyntaxAnalyzer::SyntaxAnalyzer(std::vector<Token> tokens)
    : m_tokens(std::move(tokens))
{
}

[[noreturn]] void SyntaxAnalyzer::errorExpected(const std::string& msg) const
{
    std::cerr << "[Parse Error] Expected " << msg << " on line " << peek(-1).value().line << std::endl;
    exit(EXIT_FAILURE);
}

std::optional<TermNode*> SyntaxAnalyzer::parseTerm()
{
    if (auto int_lit = tryConsume(TokenType::int_lit)) {
        auto term_int_lit = m_Allocator.construct<IntLiteralNode>(int_lit.value());
        auto term = m_Allocator.construct<TermNode>(term_int_lit);
        return term;
    }
    if (auto ident = tryConsume(TokenType::ident)) {
        auto expr_ident = m_Allocator.construct<IdentifierNode>(ident.value());
        auto term = m_Allocator.construct<TermNode>(expr_ident);
        return term;
    }
    if (const auto open_paren = tryConsume(TokenType::open_paren)) {
        auto expr = parseExpr();
        if (!expr.has_value()) {
            errorExpected("expression");
        }
        tryConsumeErr(TokenType::close_pared);
        auto term_paren = m_Allocator.construct<ParenthesizedExprNode>(expr.value());
        auto term = m_Allocator.construct<TermNode>(term_paren);
        return term;
    }
    return {};
}

std::optional<ExprNode*> SyntaxAnalyzer::parseExpr(const int min_prec)
{
    std::optional<TermNode*> term_lhs = parseTerm();
    if (!term_lhs.has_value()) {
        return {};
    }
    auto expr_lhs = m_Allocator.construct<ExprNode>(term_lhs.value());
    while (true) {
        std::optional<Token> curr_tok = peek();
        int prec;
        if (!curr_tok.has_value() || !binaryPrecedence(curr_tok->type, prec) || prec < min_prec) {
            break;
        }
        const auto [type, line, value] = consume();
        const int next_min_prec = prec + 1;
        auto expr_rhs = parseExpr(next_min_prec);
        if (!expr_rhs.has_value()) {
            errorExpected("expression");
        }
        auto expr = m_Allocator.construct<BinaryExpressionNode>();
        auto expr_lhs2 = m_Allocator.construct<ExprNode>();
        expr_lhs2->expression = expr_lhs->expression;
        switch (type) {
        case TokenType::plus:
            expr->operation = m_Allocator.construct<AdditionNode>(expr_lhs2, expr_rhs.value());
            break;
        case TokenType::star:
            expr->operation = m_Allocator.construct<MultiplicationNode>(expr_lhs2, expr_rhs.value());
            break;
        case TokenType::minus:
            expr->operation = m_Allocator.construct<SubtractionNode>(expr_lhs2, expr_rhs.value());
            break;
        case TokenType::fslash:
            expr->operation = m_Allocator.construct<DivisionNode>(expr_lhs2, expr_rhs.value());
            break;
        default:
            assert(false);
        }
        expr_lhs->expression = expr;
    }
    return expr_lhs;
}

std::optional<ScopeNode*> SyntaxAnalyzer::parseScope()
{
    if (!tryConsume(TokenType::open_curly).has_value()) {
        return {};
    }
    auto scope = m_Allocator.construct<ScopeNode>();
    while (auto stmt = parseStmt()) {
        scope->statements.push_back(stmt.value());
    }
    tryConsumeErr(TokenType::close_curly);
    return scope;
}

std::optional<ElseIfNode*> SyntaxAnalyzer::parseIfPred()
{
    if (tryConsume(TokenType::elif)) {
        tryConsumeErr(TokenType::open_paren);
        const auto elif = m_Allocator.allocate<ElseIfBranchNode>();
        if (const auto expr = parseExpr()) {
            elif->condition = expr.value();
        }
        else {
            errorExpected("expression");
        }
        tryConsumeErr(TokenType::close_pared);
        if (const auto scope = parseScope()) {
            elif->scope = scope.value();
        }
        else {
            errorExpected("scope");
        }
        elif->next = parseIfPred();
        auto pred = m_Allocator.construct<ElseIfNode>(elif);
        return pred;
    }
    if (tryConsume(TokenType::else_)) {
        auto else_ = m_Allocator.allocate<ElseBranchNode>();
        if (const auto scope = parseScope()) {
            else_->scope = scope.value();
        }
        else {
            errorExpected("scope");
        }
        auto pred = m_Allocator.construct<ElseIfNode>(else_);
        return pred;
    }
    return {};
}

std::optional<StmtNode*> SyntaxAnalyzer::parseExitStmt()
{
    consume();
    consume();
    auto stmt_exit = m_Allocator.construct<ExitStatementNode>();
    if (const auto node_expr = parseExpr()) {
        stmt_exit->expr = node_expr.value();
    }
    else {
        errorExpected("expression");
    }
    tryConsumeErr(TokenType::close_pared);
    tryConsumeErr(TokenType::semi);
    auto stmt = m_Allocator.construct<StmtNode>();
    stmt->statement = stmt_exit;
    return stmt;
}

std::optional<StmtNode*> SyntaxAnalyzer::parseLetStmt()
{
    consume();
    auto stmt_let = m_Allocator.construct<LetStatementNode>();
    stmt_let->ident = consume();
    consume();
    if (const auto expr = parseExpr()) {
        stmt_let->expr = expr.value();
    }
    else {
        errorExpected("expression");
    }
    tryConsumeErr(TokenType::semi);
    auto stmt = m_Allocator.construct<StmtNode>();
    stmt->statement = stmt_let;
    return stmt;
}

std::optional<StmtNode*> SyntaxAnalyzer::parseAssignStmt()
{
    const auto assign = m_Allocator.allocate<AssignmentStatementNode>();
    assign->ident = consume();
    consume();
    if (const auto expr = parseExpr()) {
        assign->expr = expr.value();
    }
    else {
        errorExpected("expression");
    }
    tryConsumeErr(TokenType::semi);
    auto stmt = m_Allocator.construct<StmtNode>(assign);
    return stmt;
}

std::optional<StmtNode*> SyntaxAnalyzer::parseScopeStmt()
{
    if (auto scope = parseScope()) {
        auto stmt = m_Allocator.construct<StmtNode>(scope.value());
        return stmt;
    }
    errorExpected("scope");
    return {};
}

std::optional<StmtNode*> SyntaxAnalyzer::parseIfStmt()
{
    tryConsumeErr(TokenType::open_paren);
    auto stmt_if = m_Allocator.construct<IfStatementNode>();
    if (const auto expr = parseExpr()) {
        stmt_if->condition = expr.value();
    }
    else {
        errorExpected("expression");
    }
    tryConsumeErr(TokenType::close_pared);
    if (const auto scope = parseScope()) {
        stmt_if->scope = scope.value();
    }
    else {
        errorExpected("scope");
    }
    stmt_if->pred = parseIfPred();
    auto stmt = m_Allocator.construct<StmtNode>(stmt_if);
    return stmt;
}

std::optional<StmtNode*> SyntaxAnalyzer::parseStmt()
{
    if (peek().has_value() && peek().value().type == TokenType::exit && peek(1).has_value()
        && peek(1).value().type == TokenType::open_paren) {
        return parseExitStmt();
    }
    if (peek().has_value() && peek().value().type == TokenType::let && peek(1).has_value()
        && peek(1).value().type == TokenType::ident && peek(2).has_value() && peek(2).value().type == TokenType::eq) {
        return parseLetStmt();
    }
    if (peek().has_value() && peek().value().type == TokenType::ident && peek(1).has_value()
        && peek(1).value().type == TokenType::eq) {
        return parseAssignStmt();
    }
    if (peek().has_value() && peek().value().type == TokenType::open_curly) {
        return parseScopeStmt();
    }
    if (auto if_ = tryConsume(TokenType::if_)) {
        return parseIfStmt();
    }
    return {};
}

std::optional<ProgramNode> SyntaxAnalyzer::parseProgram()
{
    ProgramNode program;
    while (peek().has_value()) {
        if (auto stmt = parseStmt()) {
            program.statements.push_back(stmt.value());
        }
        else {
            errorExpected("statement");
        }
    }
    return program;
}

[[nodiscard]] std::optional<Token> SyntaxAnalyzer::peek(const int offset) const
{
    if (m_index + offset >= m_tokens.size()) {
        return {};
    }
    return m_tokens.at(m_index + offset);
}

Token SyntaxAnalyzer::consume()
{
    auto token = m_tokens.at(m_index);
    m_index++;
    return token;
}

Token SyntaxAnalyzer::tryConsumeErr(const TokenType type)
{
    if (peek().has_value() && peek().value().type == type) {
        return consume();
    }
    errorExpected(toString(type));
    return {};
}

std::optional<Token> SyntaxAnalyzer::tryConsume(const TokenType type)
{
    if (peek().has_value() && peek().value().type == type) {
        return consume();
    }
    return {};
}