#pragma once

#include <cstdint>
#include <cstddef>
#include <sstream>
#include <string>

#include "grammar.hpp"
#include "scopes.hpp"


std::string handle_int64_immediates(uint64_t immediate);

class Generator {
public:
    explicit Generator(NodeProgram prog);

    size_t gen_term(const NodeTerm* term);
    size_t gen_bin_expr(const NodeBinExpr* bin_expr);
    size_t gen_expr(const NodeExpr* expr);
    void gen_scope(const NodeScope* scope);
    void gen_ifstmt(const NodeStmtIf* ifstmt);
    void gen_ifpred(const NodeIfPred* ifpred, const std::string end_label);
    void gen_stmt(const NodeStmt* stmt);
    std::string gen_program();

private:
    void increment_stack(int positions = 1);
    void decrement_stack(int positions = 1);
    size_t store(std::string reg, int stack_offset);
    void load(std::string reg, int stack_offset);
    void add(std::string result_reg, std::string lhs_reg, std::string rhs_reg);
    void mul(std::string result_reg, std::string lhs_reg, std::string rhs_reg);
    void sub(std::string result_reg, std::string lhs_reg, std::string rhs_reg, bool with_flags = false);
    void div(std::string result_reg, std::string lhs_reg, std::string rhs_reg);
    void cset(std::string reg, std::string condition);
    void tbnz(std::string reg, std::string bit, std::string branch_label);
    std::string get_branch_label();
    void branch(std::string branch_label);
    void add_branch(std::string branch_label);
    void _exit();

    NodeProgram m_prog;
    std::stringstream m_output;
    size_t m_stack_position = 0;
    size_t m_branch_number = 0;
    SymbolManager m_symbol_handler;
};
