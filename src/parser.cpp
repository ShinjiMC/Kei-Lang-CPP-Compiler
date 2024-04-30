#include "parser.hpp"
#include "generation.hpp"

void NodeExprIntLit::accept(Generator &generator) const
{
    generator.gen_exprIntLit(*this);
}

void NodeExprIdent::accept(Generator &generator) const
{
    generator.gen_exprIdent(*this);
}

void NodeStmtExit::accept(Generator &generator) const
{
    generator.gen_stmtExit(*this);
}

void NodeStmtLet::accept(Generator &generator) const
{
    generator.gen_stmtLet(*this);
}
