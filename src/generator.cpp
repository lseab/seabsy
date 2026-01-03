#include "generator.hpp"

#include <iomanip>
#include <iostream>


Generator::Generator(NodeProgram prog)
    : m_prog(prog)
{
    // Available temporary registers. x0 is kept free for return/exit hand-off.
    m_free_regs = {"x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8"};
}

std::string handle_int64_immediates(const uint64_t immediate, const std::string& target_reg) {
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
                output << "    movz " << target_reg << ", #";
                emit_hex16(chunk);
                if (shift != 0) add_shift(shift);
                output << "\n";
                started = true;
            }
        }
        else if (chunk != 0) {
            output << "    movk " << target_reg << ", #";
            emit_hex16(chunk);
            add_shift(shift);
            output << "\n";
        }
    }

    if (!started) {
        output << "    movz " << target_reg << ", #";
        emit_hex16(0);
        output << "\n";
    }

    return output.str();
}

std::string Generator::gen_term(const NodeTerm* term) {
    if (auto int_lit_term = std::get_if<NodeTermIntLit*>(&term->variant)) {
        Token token = (*int_lit_term)->int_lit;
        uint64_t int_value = std::stoll(token.value.value());
        std::string target_reg = acquire_reg();
        m_output << handle_int64_immediates(int_value, target_reg);
        return target_reg;
    }
    if (auto ident_term = std::get_if<NodeTermIdent*>(&term->variant)) {
        Token token = (*ident_term)->ident;
        std::string ident = token.value.value();
        std::optional<Var> var = m_symbol_handler.findSymbol(ident);
        if (!var.has_value()) {
            std::cerr << "Undefined symbol " << ident << std::endl;
            exit(EXIT_FAILURE);
        }
        std::string target_reg = acquire_reg();
        load(target_reg, 8 + (m_stack_position - var.value().stack_position) * 16);
        return target_reg;
    }
    if (auto paren_term = std::get_if<NodeTermParen*>(&term->variant)) {
        return gen_expr((*paren_term)->expr);
    }
    return "";
}

std::string Generator::gen_bin_expr(const NodeBinExpr* bin_expr) {
    std::string lhs_reg = gen_expr(bin_expr->lhs);
    std::string rhs_reg = gen_expr(bin_expr->rhs);
    switch (bin_expr->op.type) {
        case TokenType::plus:
            add(lhs_reg, lhs_reg, rhs_reg);
            break;
        case TokenType::minus:
            sub(lhs_reg, lhs_reg, rhs_reg);
            break;
        case TokenType::fslash:
            div(lhs_reg, lhs_reg, rhs_reg);
            break;
        case TokenType::star:
            mul(lhs_reg, lhs_reg, rhs_reg);
            break;
        default:
            return "";
    }
    if (lhs_reg != rhs_reg) {
        release_reg(rhs_reg);
    }
    return lhs_reg;
}

std::string Generator::gen_expr(const NodeExpr* expr) {
    if (auto term_expr = std::get_if<NodeTerm*>(&expr->variant)) {
        return gen_term((*term_expr));
    }
    if (auto bin_expr = std::get_if<NodeBinExpr*>(&expr->variant)) {
        return gen_bin_expr((*bin_expr));
    }
    return "";
}

void Generator::gen_scope(const NodeScope* scope) {
    m_symbol_handler.enterScope();
    size_t enter_stack_position = m_stack_position;
    for (NodeStmt* stmt : scope->stmts) {
        gen_stmt(stmt);
    }
    m_stack_position = enter_stack_position;
    m_symbol_handler.exitScope();
}

void Generator::gen_ifstmt(const NodeStmtIf* ifstmt) {
    std::string cond_reg = gen_expr(ifstmt->expr);
    std::string false_label = get_branch_label();
    cbz(cond_reg, false_label);
    release_reg(cond_reg);
    gen_scope(ifstmt->scope);
    if (ifstmt->pred.has_value()) {
        const std::string end_label = get_branch_label();
        branch(end_label);
        add_branch(false_label);
        gen_ifpred(ifstmt->pred.value(), end_label);
        add_branch(end_label);
    }
    else {
        branch(false_label);
        add_branch(false_label);
    }
}

void Generator::gen_ifpred(const NodeIfPred* ifpred, const std::string end_label) {
    if (auto ifpred_elif = std::get_if<NodeStmtIf*>(&ifpred->variant)) {
        auto ifstmt = (*ifpred_elif);
        std::string cond_reg = gen_expr(ifstmt->expr);
        if (ifstmt->pred.has_value()) {
            std::string false_label = get_branch_label();
            cbz(cond_reg, false_label);
            release_reg(cond_reg);
            gen_scope(ifstmt->scope);
            branch(end_label);
            add_branch(false_label);
            gen_ifpred(ifstmt->pred.value(), end_label);
        }
        else {
            cbz(cond_reg, end_label);
            release_reg(cond_reg);
            gen_scope(ifstmt->scope);
            branch(end_label);
        }
    }
    else if (auto ifpred_else = std::get_if<NodeIfPredElse*>(&ifpred->variant)) {
        gen_scope((*ifpred_else)->scope);
        branch(end_label);
    }
}

void Generator::gen_stmt(const NodeStmt* stmt) {
    if (auto stmt_return = std::get_if<NodeStmtReturn*>(&stmt->variant)) {
        std::string result_reg = gen_expr((*stmt_return)->expr);
        decrement_stack(m_stack_position);
        if (result_reg != "x0") {
            m_output << "    mov x0, " << result_reg << "\n";
        }
        release_reg(result_reg);
        m_output << "    ret\n";
        return;
    }
    if (auto stmt_exit = std::get_if<NodeStmtExit*>(&stmt->variant)) {
        std::string result_reg = gen_expr((*stmt_exit)->expr);
        if (result_reg != "x0") {
            m_output << "    mov x0, " << result_reg << "\n";
        }
        decrement_stack(m_stack_position);
        release_reg(result_reg);
        _exit();
        return;
    }
    if (auto stmt_let = std::get_if<NodeStmtLet*>(&stmt->variant)) {
        std::string ident = (*stmt_let)->ident.value.value();
        std::string result_reg = gen_expr((*stmt_let)->expr);
        increment_stack();
        store(result_reg, 8);
        m_symbol_handler.declareSymbol(ident, m_stack_position);
        release_reg(result_reg);
        return;
    }
    if (auto stmt_assign = std::get_if<NodeStmtAssign*>(&stmt->variant)) {
        std::string ident = (*stmt_assign)->ident.value.value();
        if (auto var = m_symbol_handler.findSymbol(ident)) {
            std::string result_reg = gen_expr((*stmt_assign)->expr);
            store(result_reg, 8 + (m_stack_position - var.value().stack_position) * 16);
            release_reg(result_reg);
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

void Generator::cbz(std::string cond_reg, std::string branch_label) {
    m_output << "    cbz " << cond_reg << ", " << branch_label << "\n";
}

std::string Generator::acquire_reg() {
    if (m_free_regs.empty()) {
        std::cerr << "Register exhaustion during code generation" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::string reg = m_free_regs.back();
    m_free_regs.pop_back();
    return reg;
}

void Generator::release_reg(const std::string& reg) {
    m_free_regs.push_back(reg);
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
