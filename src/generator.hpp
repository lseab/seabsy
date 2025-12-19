#include <iomanip>
#include <sstream>
#include <string>
#include <iostream>
#include <variant>

#include "parsing.hpp"


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
            Var var = var_map[ident];
            load("x0", 8 + (m_stack_position - var.stack_position) * 16);
            increment_stack();
            return store("x0", 8);
        }
        return {};
    }

    inline size_t gen_bin_expr(const NodeBinExpr* bin_expr) {
        if (auto add_bin_expr = std::get_if<NodeBinExprAdd*>(&bin_expr->bin_expr)) {
            size_t stack_pos_l = gen_expr((*add_bin_expr)->lhs);
            size_t stack_pos_r = gen_expr((*add_bin_expr)->rhs);
            // pop the last
            load("x7", 8 + (m_stack_position - stack_pos_l) * 16);
            load("x8", 8 + (m_stack_position - stack_pos_r) * 16);
            // add values
            add("x0", "x7", "x8");
            // push result to stack
            increment_stack();
            return store("x0", 8);
        }
        else if (auto multi_bin_expr = std::get_if<NodeBinExprMulti*>(&bin_expr->bin_expr)) {
            size_t stack_pos_l = gen_expr((*multi_bin_expr)->lhs);
            size_t stack_pos_r = gen_expr((*multi_bin_expr)->rhs);
            // pop the last
            load("x7", 8 + (m_stack_position - stack_pos_l) * 16);
            load("x8", 8 + (m_stack_position - stack_pos_r) * 16);
            // add values
            mul("x0", "x7", "x8");
            // push result to stack
            increment_stack();
            return store("x0", 8);
        }
        else if (auto sub_bin_expr = std::get_if<NodeBinExprSub*>(&bin_expr->bin_expr)) {
            size_t stack_pos_l = gen_expr((*sub_bin_expr)->lhs);
            size_t stack_pos_r = gen_expr((*sub_bin_expr)->rhs);
            // pop the last
            load("x7", 8 + (m_stack_position - stack_pos_l) * 16);
            load("x8", 8 + (m_stack_position - stack_pos_r) * 16);
            // add values
            sub("x0", "x7", "x8");
            // push result to stack
            increment_stack();
            return store("x0", 8);
        }
        return {};
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

    inline void gen_stmt(const NodeStmt* stmt) {
        if (auto stmt_return = std::get_if<NodeStmtReturn*>(&stmt->stmt)) {
            gen_expr((*stmt_return)->expr);
            decrement_stack(m_stack_position);
            m_output << "    ret\n";
        }
        else if (auto stmt_let = std::get_if<NodeStmtLet*>(&stmt->stmt)) {
            std::string ident = (*stmt_let)->ident.value.value();
            if (var_map.contains(ident)) {
                std::cerr << "Redefinition of " << ident << std::endl;
                exit(EXIT_FAILURE);
            }
            else {
                gen_expr((*stmt_let)->expr);
                var_map[ident] = Var{.ident = ident, .stack_position = m_stack_position};
            }
        }
    }

    inline std::string gen_program() {
        m_output << ".globl _main\n.p2align 2\n_main:\n";
        for (NodeStmt* stmt: m_prog.stmts) {
            gen_stmt(stmt);
        }
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

    void sub(std::string result_reg, std::string lhs_reg, std::string rhs_reg) {
        m_output << "    sub " << result_reg << ", " << lhs_reg << ", " << rhs_reg << "\n";
    }

    struct Var {
        std::string ident;
        size_t stack_position;
    };

    NodeProgram m_prog;
    std::stringstream m_output;
    size_t m_stack_position = 0;
    std::unordered_map<std::string, Var> var_map;
};