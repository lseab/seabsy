#include <sstream>
#include <string>
#include <iostream>
#include <optional>

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
    inline Generator(NodeReturn root) :
        m_root(root)
    {
    }
    inline std::string generate() {
        std::stringstream output;
        output << ".globl _main\n.p2align 2\n_main:\n";
        std::optional<Token> token = m_root.expr.int_lit;
        uint64_t int_value = std::stoll(token->value.value());
        std::string immediate_output = handle_int64_immediates(int_value);
        output << immediate_output;
        output << "    ret\n";
        return output.str();
    }

private:
    NodeReturn m_root;
};