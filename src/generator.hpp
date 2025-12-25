#include <iomanip>
#include <sstream>
#include <string>
#include <iostream>
#include <variant>

#include "parsing.hpp"
#include "scopes.hpp"


inline std::string handle_int64_immediates(uint64_t immediate) {
    std::stringstream output;
    bool started = false;

    auto emit_hex16 = [&output](uint16_t chunk) {
        // Always print exactly 4 hex digits for a 16-bit chunk.
        output << "0x"
               << std::hex
               << std::setw(4) << std::setfill('0')
               << chunk
               << std::dec;
    };

    auto add_shift = [&](int shift) {
        output << ", lsl #" << shift;
    };

    for (int i = 0; i < 4; i++) {
        uint16_t chunk = static_cast<uint16_t>((immediate >> (i * 16)) & 0xFFFF);
        int shift = i * 16;
        if (!started) {
            if (chunk != 0) {
                output << "    movz x0, #";
                emit_hex16(chunk);
                if (shift != 0) add_shift(shift);
                output << "\n";
                started = true;
            }
        }
        else if (chunk != 0) {
            output << "    movk x0, #";
            emit_hex16(chunk);
            add_shift(shift);
            output << "\n";
        }
    }

    // If the immediate is 0, emit a single movz x0, #0
    if (!started) {
        output << "    movz x0, #";
        emit_hex16(0);
        output << "\n";
    }

    return output.str();
}


class Generator {
public:
    inline Generator(NodeProgram prog) :
        m_prog(prog)
    {
    }

    inline size_t gen_term(const NodeTerm* term) {
        if (auto int_lit_term = std::get_if<NodeTermIntLit*>(&term->term)) {
            Token token = (*int_lit_term)->int_lit;
            uint64_t int_value = std::stoll(token.value.value());
            std::string immediate_output = handle_int64_immediates(int_value);
            m_output << immediate_output;
            increment_stack();
            return store("x0", 8);
        }
        else if (auto ident_term = std::get_if<NodeTermIdent*>(&term->term)) {
            Token token = (*ident_term)->ident;
            std::string ident = token.value.value();
            std::optional<Var> var = m_symbol_handler.findSymbol(ident);
            if (!var.has_value()) {
                std::cerr << "Undefined symbol " << ident << std::endl;
                exit(EXIT_FAILURE);
            }
            load("x0", 8 + (m_stack_position - var.value().stack_position) * 16);
            increment_stack();
            return store("x0", 8);
        }
        else if (auto paren_term = std::get_if<NodeTermParen*>(&term->term)) {
            return gen_expr((*paren_term)->expr);
        }
        return {};
    }

    inline size_t gen_bin_expr(const NodeBinExpr* bin_expr) {
        size_t stack_pos_l = gen_expr(bin_expr->lhs);
        size_t stack_pos_r = gen_expr(bin_expr->rhs);
        // pop the last
        load("x7", 8 + (m_stack_position - stack_pos_l) * 16);
        load("x8", 8 + (m_stack_position - stack_pos_r) * 16);
        switch(bin_expr->op.type) {
            case TokenType::plus:
                add("x0", "x7", "x8");
                break;
            case TokenType::minus:
                sub("x0", "x7", "x8");
                break;
            case TokenType::fslash:
                div("x0", "x7", "x8");
                break;
            case TokenType::star:
                mul("x0", "x7", "x8");
                break;
            default:
                return {};
        }
        // push result to stack
        increment_stack();
        return store("x0", 8);
    }

    inline size_t gen_expr(const NodeExpr* expr) {
        if (auto term_expr = std::get_if<NodeTerm*>(&expr->expr)) {
            return gen_term((*term_expr));
        }
        else if (auto bin_expr = std::get_if<NodeBinExpr*>(&expr->expr)) {
            return gen_bin_expr((*bin_expr));
        }
        return {};
    }

    inline void gen_scope(const NodeScope* scope) {
        m_symbol_handler.enterScope();
        for (NodeStmt* stmt: scope->stmts) {
            gen_stmt(stmt);
        }
        m_symbol_handler.exitScope();
    }

    inline void gen_ifstmt(const NodeStmtIf* ifstmt) {
        size_t stack_pos = gen_expr(ifstmt->expr);
        // load expression result into x8
        load("x8", 8 + (m_stack_position - stack_pos) * 16);
        // Compare result to 0
        sub("x8", "x8", "#0", true);
        // Override x8 to 1 if the subtraction result is 0, ie if x8 is 0
        cset("x8", "eq");
        std::string true_branch = get_branch_label();
        std::string false_branch = get_branch_label();
        // set the branching instructions
        tbnz("w8", "#0", false_branch);
        branch(true_branch);
        // define the branches, making sure to reset the stack pointer
        // after generating the assembly for a given branch
        add_branch(true_branch);
        size_t stack_pos_before_branch = m_stack_position;
        gen_scope(ifstmt->scope);
        add_branch(false_branch);
        m_stack_position = stack_pos_before_branch;
    }

    inline void gen_stmt(const NodeStmt* stmt) {
        if (auto stmt_return = std::get_if<NodeStmtReturn*>(&stmt->stmt)) {
            gen_expr((*stmt_return)->expr);
            decrement_stack(m_stack_position);
            m_output << "    ret\n";
        }
        else if (auto stmt_exit = std::get_if<NodeStmtExit*>(&stmt->stmt)) {
            size_t stack_pos = gen_expr((*stmt_exit)->expr);
            decrement_stack(m_stack_position);
            load("x0", 8 + (m_stack_position - stack_pos) * 16);
            _exit();
        }
        else if (auto stmt_let = std::get_if<NodeStmtLet*>(&stmt->stmt)) {
            std::string ident = (*stmt_let)->ident.value.value();
            if (m_symbol_handler.findSymbol(ident)) {
                std::cerr << "Redefinition of " << ident << std::endl;
                exit(EXIT_FAILURE);
            }
            else {
                gen_expr((*stmt_let)->expr);
                m_symbol_handler.declareSymbol(ident, m_stack_position);
            }
        }
        else if (auto scope = std::get_if<NodeScope*>(&stmt->stmt)) {
            gen_scope((*scope));
        }
        else if (auto stmt_if = std::get_if<NodeStmtIf*>(&stmt->stmt)) {
            gen_ifstmt((*stmt_if));
        }
    }

    inline std::string gen_program() {
        m_output << ".globl _main\n.p2align 2\n_main:\n";
        for (NodeStmt* stmt: m_prog.stmts) {
            gen_stmt(stmt);
        }
        m_output << "    mov x0, #0\n";
        _exit();
        return m_output.str();
    }

private:
    void increment_stack(int positions = 1) {
        m_output << "    sub sp, sp, #" << positions * 16 << "\n";
        m_stack_position += positions;
    }

    void decrement_stack(int positions = 1) {
        m_output << "    add sp, sp, #" << positions * 16 << "\n";
        m_stack_position -= positions;
    }

    size_t store(std::string reg, int stack_offset) {
        // Stores the value at a given register on the stack (with a given offset)
        // Returns the stack position at the time of storing
        m_output << "    str " << reg << ", [sp, #" << stack_offset << "]\n";
        return m_stack_position;
    }

    void load(std::string reg, int stack_offset) {
        m_output << "    ldr " << reg << ", [sp, #" << stack_offset << "]\n";
    }

    void add(std::string result_reg, std::string lhs_reg, std::string rhs_reg) {
        m_output << "    add " << result_reg << ", " << lhs_reg << ", " << rhs_reg << "\n";
    }

    void mul(std::string result_reg, std::string lhs_reg, std::string rhs_reg) {
        m_output << "    mul " << result_reg << ", " << lhs_reg << ", " << rhs_reg << "\n";
    }

    void sub(std::string result_reg, std::string lhs_reg, std::string rhs_reg, bool with_flags = false) {
        if (with_flags) {
            m_output << "    subs ";
        }
        else {
            m_output << "    sub ";
        }
        m_output << result_reg << ", " << lhs_reg << ", " << rhs_reg << "\n";
    }

    void div(std::string result_reg, std::string lhs_reg, std::string rhs_reg) {
        m_output << "    sdiv " << result_reg << ", " << lhs_reg << ", " << rhs_reg << "\n";
    }

    void cset(std::string reg, std::string condition) {
        m_output << "    cset " << reg << ", " << condition << "\n";
    }

    void tbnz(std::string reg, std::string bit, std::string branch_label) {
        m_output << "    tbnz " << reg << ", " << bit << ", " << branch_label << "\n";
    }

    std::string get_branch_label() {
        m_branch_number++;
        return "LBB0_" + std::to_string(m_branch_number);
    }

    void branch(std::string branch_label) {
        m_output << "    b " << branch_label << "\n";
    }

    void add_branch(std::string branch_label) {
        m_output << branch_label << ":\n";
    }

    void _exit() {
        m_output << "    bl _exit\n";
    }

    NodeProgram m_prog;
    std::stringstream m_output;
    size_t m_stack_position = 0;
    size_t m_branch_number = 0;
    SymbolManager m_symbol_handler;
};