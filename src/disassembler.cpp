#include "disassembler.hh"
#include <cinttypes>
#include <cstdio>
#include <ostream>
#include <sstream>
#include "output.hh"

using std::cerr;
using std::ostringstream;
using std::pair;

#define STRING_NAME(x) \
    case operation::x: \
        return #x

std::string disassembler::to_str(const instruction& i) noexcept {
    using operation = instruction::operation;

    switch (i.m_op) {
        STRING_NAME(NOP);
        STRING_NAME(ADD);
        STRING_NAME(SUB);
        STRING_NAME(MUL);
        STRING_NAME(DIV);
        STRING_NAME(MOD);
        STRING_NAME(POP);
        STRING_NAME(EXT);
        STRING_NAME(INC);
        STRING_NAME(DEC);
        STRING_NAME(AND);
        STRING_NAME(IOR);
        STRING_NAME(XOR);
        STRING_NAME(NOT);
        STRING_NAME(GTC);
        STRING_NAME(PTC);
        STRING_NAME(GTI);
        STRING_NAME(PTI);
        STRING_NAME(IDX);
        STRING_NAME(DUP);
        STRING_NAME(RND);
        STRING_NAME(EXP);
        STRING_NAME(SWP);
        STRING_NAME(GTM);
        STRING_NAME(GDT);
        STRING_NAME(DP2);
        STRING_NAME(TKL);
        STRING_NAME(TJN);
        case operation::PSI: {
            int24_t value = i.m_arg.number;
            ostringstream result;
            result << "PSI #";
            print_unichar(value, result);
            return result.str();
        }
        case operation::PSC: {
            int24_t value = i.m_arg.number;
            ostringstream result;
            result << "PSC '";
            print_unichar(value, result);
            result << "' ; 0x";
            char buf[7];
            snprintf(buf, sizeof buf, "%" PRIx32, static_cast<uint32_t>(value));
            result << buf;
            return result.str();
        }
        case operation::JMP: {
            pair<size_t, size_t> target = i.m_arg.next;
            ostringstream result;
            result << "JMP " << target.first << "." << target.second;
            return result.str();
        }
        case operation::BNG: {
            pair<size_t, size_t> target = i.m_arg.choice.second;
            ostringstream result;
            result << "BNG " << target.first << "." << target.second;
            return result.str();
        }
        case operation::TSP: {
            pair<size_t, size_t> target = i.m_arg.choice.second;
            ostringstream result;
            result << "TSP " << target.first << "." << target.second;
            return result.str();
        }
        default:
            cerr << "Unrecognized opcode '";
            print_unichar(static_cast<int24_t>(i.m_op), cerr);
            cerr << '\'' << std::endl;
            exit(EXIT_FAILURE);
    }
}

void disassembler::write_state(std::ostream& os) {
    if (m_fragments == nullptr) {
        build_state();
    }

    for (size_t i = 0; i < m_fragments->size(); ++i) {
        const std::vector<instruction>& frag = *m_fragments->at(i);
        for (size_t j = 0; j < frag.size(); ++j) {
            // skip NOPs if requested
            const instruction& ins = frag[j];
            if (m_flags.hide_nops && ins.is_nop()) {
                continue;
            }

            // write out the label
            os << i << '.' << j << ":\t";
            // write out the instruction name
            os << to_str(ins);

            // if it's a branch followed by a jump, write that
            const std::pair<size_t, size_t>* next = ins.first_if_branch();
            if (next != nullptr && (next->first != i + 1 || next->second != 0)) {
                os << "\n\tJMP " << next->first << '.' << next->second;
            }

            // write the newline
            os << '\n';
        }
    }

    // Flush the buffer, if applicable
    os << std::flush;
}
