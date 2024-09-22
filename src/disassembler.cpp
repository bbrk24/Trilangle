#include "disassembler.hh"
#include <cinttypes>
#include <cstdlib>

using std::cerr;
using std::ostringstream;
using std::pair;

#define STRING_NAME(x) \
    case operation::x: \
        os << #x; \
        return

void disassembler::to_str(const instruction& i, std::ostream& os) {
    using operation = instruction::operation;

    switch (i.get_op()) {
        STRING_NAME(NOP);
        STRING_NAME(ADD);
        STRING_NAME(SUB);
        STRING_NAME(MUL);
        STRING_NAME(DIV);
        STRING_NAME(UDV);
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
        STRING_NAME(PTU);
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
            int24_t value = i.get_arg().number;
            os << "PSI #";
            print_unichar(value, os);
            return;
        }
        case operation::PSC: {
            int24_t value = i.get_arg().number;
            os << "PSC '";
            print_unichar(value, os);
            os << "' ; 0x";
            char buf[7];
            snprintf(buf, sizeof buf, "%" PRIx32, static_cast<uint32_t>(value));
            os << buf;
            return;
        }
        case operation::JMP: {
            pair<size_t, size_t> target = i.get_arg().next;
            os << "JMP " << target.first << "." << target.second;
            return;
        }
        case operation::BNG: {
            pair<size_t, size_t> target = i.get_arg().choice.second;
            os << "BNG " << target.first << "." << target.second;
            return;
        }
        case operation::TSP: {
            pair<size_t, size_t> target = i.get_arg().choice.second;
            os << "TSP " << target.first << "." << target.second;
            return;
        }
        default:
            cerr << "Unrecognized opcode '";
            print_unichar(static_cast<int24_t>(i.get_op()), cerr);
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
            to_str(ins, os);

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
