#pragma once

#include <cassert>
#include <variant>

#include "Components/lexing/lexicalAnalyzer.hpp"
#include "Components/memory/memoryAllocator.hpp"

struct ExprNode;

struct IntLiteralNode {
    Token intLit;
};

struct IdentifierNode {
    Token ident;
};

struct ParenthesizedExprNode {
    ExprNode* expr;
};

struct AdditionNode {
    ExprNode* lhs;
    ExprNode* rhs;
};

struct MultiplicationNode {
    ExprNode* lhs;
    ExprNode* rhs;
};

struct SubtractionNode {
    ExprNode* lhs;
    ExprNode* rhs;
};

struct DivisionNode {
    ExprNode* lhs;
    ExprNode* rhs;
};

struct BinaryExpressionNode {
    std::variant<AdditionNode*, MultiplicationNode*, SubtractionNode*, DivisionNode*> operation;
};

struct TermNode {
    std::variant<IntLiteralNode*, IdentifierNode*, ParenthesizedExprNode*> term;
};

struct ExprNode {
    std::variant<TermNode*, BinaryExpressionNode*> expression;
};

struct ExitStatementNode {
    ExprNode* expr;
};

struct LetStatementNode {
    Token ident;
    ExprNode* expr {};
};

struct StmtNode;

struct ScopeNode {
    std::vector<StmtNode*> statements;
};

struct ElseIfNode;

struct ElseIfBranchNode {
    ExprNode* condition {};
    ScopeNode* scope {};
    std::optional<ElseIfNode*> next;
};

struct ElseBranchNode {
    ScopeNode* scope;
};

struct ElseIfNode {
    std::variant<ElseIfBranchNode*, ElseBranchNode*> branch;
};

struct IfStatementNode {
    ExprNode* condition {};
    ScopeNode* scope {};
    std::optional<ElseIfNode*> pred;
};

struct AssignmentStatementNode {
    Token ident;
    ExprNode* expr {};
};

struct StmtNode {
    std::variant<ExitStatementNode*, LetStatementNode*, ScopeNode*, IfStatementNode*, AssignmentStatementNode*>
        statement;
};

struct ProgramNode {
    std::vector<StmtNode*> statements;
};

class SyntaxAnalyzer {
private:
    const std::vector<Token> m_tokens;
    size_t m_index = 0;
    static constexpr size_t defaultAllocatorSize = 1024 * 1024 * 4; // 4 MB default size
    MemoryAllocator m_Allocator { defaultAllocatorSize };
    [[nodiscard]] std::optional<Token> peek(const int offset = 0) const;
    Token consume();
    Token tryConsumeErr(const TokenType type);
    std::optional<Token> tryConsume(const TokenType type);

public:
    explicit SyntaxAnalyzer(std::vector<Token> tokens);
    [[noreturn]] void errorExpected(const std::string& msg) const;
    std::optional<TermNode*> parseTerm();
    std::optional<ExprNode*> parseExpr(const int min_prec = 0);
    std::optional<ScopeNode*> parseScope();
    std::optional<ElseIfNode*> parseIfPred();
    std::optional<StmtNode*> parseExitStmt();
    std::optional<StmtNode*> parseLetStmt();
    std::optional<StmtNode*> parseAssignStmt();
    std::optional<StmtNode*> parseScopeStmt();
    std::optional<StmtNode*> parseIfStmt();
    std::optional<StmtNode*> parseStmt();
    std::optional<ProgramNode> parseProgram();
};