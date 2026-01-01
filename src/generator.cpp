#include "generator.hpp"

#include <iomanip>
#include <iostream>


Generator::Generator(NodeProgram prog)
    : m_prog(prog)
{
}

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

    if (!started) {
        output << "    movz x0, #";
        emit_hex16(0);
        output << "\n";
    }

    return output.str();
}

size_t Generator::gen_term(const NodeTerm* term) {
    if (auto int_lit_term = std::get_if<NodeTermIntLit*>(&term->variant)) {
        Token token = (*int_lit_term)->int_lit;
        uint64_t int_value = std::stoll(token.value.value());
        std::string immediate_output = handle_int64_immediates(int_value);
        m_output << immediate_output;
        increment_stack();
        return store("x0", 8);
    }
    if (auto ident_term = std::get_if<NodeTermIdent*>(&term->variant)) {
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
    if (auto paren_term = std::get_if<NodeTermParen*>(&term->variant)) {
        return gen_expr((*paren_term)->expr);
    }
    return {};
}

size_t Generator::gen_bin_expr(const NodeBinExpr* bin_expr) {
    size_t stack_pos_l = gen_expr(bin_expr->lhs);
    size_t stack_pos_r = gen_expr(bin_expr->rhs);
    load("x7", 8 + (m_stack_position - stack_pos_l) * 16);
    load("x8", 8 + (m_stack_position - stack_pos_r) * 16);
    switch (bin_expr->op.type) {
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
    increment_stack();
    return store("x0", 8);
}

size_t Generator::gen_expr(const NodeExpr* expr) {
    if (auto term_expr = std::get_if<NodeTerm*>(&expr->variant)) {
        return gen_term((*term_expr));
    }
    if (auto bin_expr = std::get_if<NodeBinExpr*>(&expr->variant)) {
        return gen_bin_expr((*bin_expr));
    }
    return {};
}

void Generator::gen_scope(const NodeScope* scope) {
    m_symbol_handler.enterScope();
    for (NodeStmt* stmt : scope->stmts) {
        gen_stmt(stmt);
    }
    m_symbol_handler.exitScope();
}

void Generator::gen_ifstmt(const NodeStmtIf* ifstmt) {
    size_t stack_pos = gen_expr(ifstmt->expr);
    load("x8", 8 + (m_stack_position - stack_pos) * 16);
    sub("x8", "x8", "#0", true);
    cset("x8", "eq");
    std::string true_branch = get_branch_label();
    std::string false_branch = get_branch_label();
    tbnz("w8", "#0", false_branch);
    branch(true_branch);
    add_branch(true_branch);
    size_t stack_pos_before_branch = m_stack_position;
    gen_scope(ifstmt->scope);
    add_branch(false_branch);
    m_stack_position = stack_pos_before_branch;
    if (ifstmt->pred.has_value()) {
        gen_ifpred(ifstmt->pred.value());
    }
}

void Generator::gen_ifpred(const NodeIfPred* ifpred) {
    if (auto ifpred_elif = std::get_if<NodeStmtIf*>(&ifpred->variant)) {
        gen_ifstmt((*ifpred_elif));
    }
    else if (auto ifpred_else = std::get_if<NodeIfPredElse*>(&ifpred->variant)) {
        gen_scope((*ifpred_else)->scope);
    }
}

void Generator::gen_stmt(const NodeStmt* stmt) {
    if (auto stmt_return = std::get_if<NodeStmtReturn*>(&stmt->variant)) {
        gen_expr((*stmt_return)->expr);
        decrement_stack(m_stack_position);
        m_output << "    ret\n";
        return;
    }
    if (auto stmt_exit = std::get_if<NodeStmtExit*>(&stmt->variant)) {
        size_t stack_pos = gen_expr((*stmt_exit)->expr);
        decrement_stack(m_stack_position);
        load("x0", 8 + (m_stack_position - stack_pos) * 16);
        _exit();
        return;
    }
    if (auto stmt_let = std::get_if<NodeStmtLet*>(&stmt->variant)) {
        std::string ident = (*stmt_let)->ident.value.value();
        if (m_symbol_handler.findSymbol(ident)) {
            std::cerr << "Redefinition of " << ident << std::endl;
            exit(EXIT_FAILURE);
        }
        gen_expr((*stmt_let)->expr);
        m_symbol_handler.declareSymbol(ident, m_stack_position);
        return;
    }
    if (auto stmt_assign = std::get_if<NodeStmtAssign*>(&stmt->variant)) {
        std::string ident = (*stmt_assign)->ident.value.value();
        if (auto var = m_symbol_handler.findSymbol(ident)) {
            size_t stack_pos = gen_expr((*stmt_assign)->expr);
            load("x0", 8 + (m_stack_position - stack_pos) * 16);
            store("x0", 8 + (m_stack_position - var.value().stack_position) * 16);
        }
        else {
            std::cerr << "Undeclared identifier " << ident << std::endl;
            exit(EXIT_FAILURE);
        }
        return;
    }
    if (auto scope = std::get_if<NodeScope*>(&stmt->variant)) {
        gen_scope((*scope));
        return;
    }
    if (auto stmt_if = std::get_if<NodeStmtIf*>(&stmt->variant)) {
        gen_ifstmt((*stmt_if));
    }
}

std::string Generator::gen_program() {
    m_output << ".globl _main\n.p2align 2\n_main:\n";
    for (NodeStmt* stmt : m_prog.stmts) {
        gen_stmt(stmt);
    }
    m_output << "    mov x0, #0\n";
    _exit();
    return m_output.str();
}

void Generator::increment_stack(int positions) {
    m_output << "    sub sp, sp, #" << positions * 16 << "\n";
    m_stack_position += positions;
}

void Generator::decrement_stack(int positions) {
    m_output << "    add sp, sp, #" << positions * 16 << "\n";
    m_stack_position -= positions;
}

size_t Generator::store(std::string reg, int stack_offset) {
    m_output << "    str " << reg << ", [sp, #" << stack_offset << "]\n";
    return m_stack_position;
}

void Generator::load(std::string reg, int stack_offset) {
    m_output << "    ldr " << reg << ", [sp, #" << stack_offset << "]\n";
}

void Generator::add(std::string result_reg, std::string lhs_reg, std::string rhs_reg) {
    m_output << "    add " << result_reg << ", " << lhs_reg << ", " << rhs_reg << "\n";
}

void Generator::mul(std::string result_reg, std::string lhs_reg, std::string rhs_reg) {
    m_output << "    mul " << result_reg << ", " << lhs_reg << ", " << rhs_reg << "\n";
}

void Generator::sub(std::string result_reg, std::string lhs_reg, std::string rhs_reg, bool with_flags) {
    if (with_flags) {
        m_output << "    subs ";
    }
    else {
        m_output << "    sub ";
    }
    m_output << result_reg << ", " << lhs_reg << ", " << rhs_reg << "\n";
}

void Generator::div(std::string result_reg, std::string lhs_reg, std::string rhs_reg) {
    m_output << "    sdiv " << result_reg << ", " << lhs_reg << ", " << rhs_reg << "\n";
}

void Generator::cset(std::string reg, std::string condition) {
    m_output << "    cset " << reg << ", " << condition << "\n";
}

void Generator::tbnz(std::string reg, std::string bit, std::string branch_label) {
    m_output << "    tbnz " << reg << ", " << bit << ", " << branch_label << "\n";
}

std::string Generator::get_branch_label() {
    m_branch_number++;
    return "LBB0_" + std::to_string(m_branch_number);
}

void Generator::branch(std::string branch_label) {
    m_output << "    b " << branch_label << "\n";
}

void Generator::add_branch(std::string branch_label) {
    m_output << branch_label << ":\n";
}

void Generator::_exit() {
    m_output << "    bl _exit\n";
}
