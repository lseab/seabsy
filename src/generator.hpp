#include <sstream>
#include <string>
#include <iostream>
#include <variant>

#include "parsing.hpp"


std::string handle_int64_immediates(uint64_t immediate) {
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

    inline void gen_expr(const NodeExpr expr) {
        if (auto value = std::get_if<NodeExprIntLit>(&expr.expr)) {
            NodeExprIntLit int_lit_expr = std::get<NodeExprIntLit>(expr.expr);
            std::optional<Token> token = int_lit_expr.int_lit;
            uint64_t int_value = std::stoll(token->value.value());
            std::string immediate_output = handle_int64_immediates(int_value);
            m_output << immediate_output;
            increment_stack();
            store("x0", 8);
        }
        else if (auto value = std::get_if<NodeExprIdent>(&expr.expr)) {
        }
    }

    inline void gen_stmt(const NodeStmt stmt) {
        if (auto value = std::get_if<NodeStmtReturn>(&stmt.stmt)) {
            NodeStmtReturn stmt_return = std::get<NodeStmtReturn>(stmt.stmt);
            gen_expr(stmt_return.expr);
            m_output << "    ret\n";
        }
        else if (auto value = std::get_if<NodeStmtLet>(&stmt.stmt)) {
            NodeStmtLet stmt_let = std::get<NodeStmtLet>(stmt.stmt);
            gen_expr(stmt_let.expr);
        }
    }

    inline std::string gen_program() {
        m_output << ".globl _main\n.p2align 2\n_main:\n";
        for (const NodeStmt stmt: m_prog.stmts) {
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

    void store(std::string reg, int stack_offset) {
        m_output << "    str " << reg << ", [sp, #" << stack_offset << "]\n";
    }

    void load(std::string reg, int stack_offset) {
        m_output << "    ldr " << reg << ", [sp, #" << stack_offset << "]\n";
    }

    NodeProgram m_prog;
    std::stringstream m_output;
    size_t m_stack_position = 0;
};