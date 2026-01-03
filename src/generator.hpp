#pragma once

#include <cstdint>
#include <cstddef>
#include <sstream>
#include <string>
#include <vector>

#include "grammar.hpp"
#include "scopes.hpp"


std::string handle_int64_immediates(const uint64_t immediate, const std::string& target_reg);

class Generator {
public:
    explicit Generator(NodeProgram prog);

    std::string gen_term(const NodeTerm* term);
    std::string gen_bin_expr(const NodeBinExpr* bin_expr);
    std::string gen_expr(const NodeExpr* expr);
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
    void cbz(std::string cond_reg, std::string branch_label);
    std::string acquire_reg();
    void release_reg(const std::string& reg);
    std::string get_branch_label();
    void branch(std::string branch_label);
    void add_branch(std::string branch_label);
    void _exit();

    NodeProgram m_prog;
    std::stringstream m_output;
    size_t m_stack_position = 0;
    size_t m_branch_number = 0;
    SymbolManager m_symbol_handler;
    std::vector<std::string> m_free_regs;
};
