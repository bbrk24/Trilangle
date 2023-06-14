#include "instruction.hh"
#include <cinttypes>
#include <cstdio>
#include <sstream>
#include "output.hh"

using std::cerr;
using std::ostringstream;

#define STRING_NAME(x) \
    case operation::x: \
        return #x

instruction::instruction(instruction_pointer ip, const program& program) noexcept : m_op(operation::NOP), m_arg() {
    int24_t op = program.at(ip.coords.first, ip.coords.second);
    assert(!is_branch(op, ip.dir));
    switch (op) {
        // Threads & branching first
        case THR_E:
            switch (ip.dir) {
                case direction::east:
                    m_op = operation::TKL;
                    break;
                case direction::northeast:
                case direction::southeast:
                    m_op = operation::TJN;
                    break;
                case direction::west:
                    unreachable("Already asserted above");
                default:
                    break;
            }
            break;
        case THR_W:
            switch (ip.dir) {
                case direction::west:
                    m_op = operation::TKL;
                    break;
                case direction::northwest:
                case direction::southwest:
                    m_op = operation::TJN;
                    break;
                case direction::east:
                    unreachable("Already asserted above");
                default:
                    break;
            }
            break;
        // These are effectively NOPs
        case BNG_E:
        case BNG_NE:
        case BNG_NW:
        case BNG_SE:
        case BNG_SW:
        case BNG_W:
        case MIR_EW:
        case MIR_NESW:
        case MIR_NS:
        case MIR_NWSE:
        case SKP:
            break;
        // These are direction-insensitive but take an argument
        case PSI:
        case PSC:
            program_walker::advance(ip, program.side_length());
            m_arg.number = program.at(ip.coords.first, ip.coords.second);
            FALLTHROUGH
        // Most things are direction-insensitive and take no arguments
        default:
            m_op = static_cast<operation>(op);
    }
}

std::string instruction::to_str() const noexcept {
    switch (m_op) {
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
            int24_t value = m_arg.number;
            ostringstream result;
            result << "PSI #";
            printunichar(value, result);
            return result.str();
        }
        case operation::PSC: {
            int24_t value = m_arg.number;
            ostringstream result;
            result << "PSC '";
            printunichar(value, result);
            result << "' ; 0x";
            char buf[7];
            snprintf(buf, sizeof buf, "%" PRIx32, static_cast<uint32_t>(value));
            result << buf;
            return result.str();
        }
        case operation::JMP: {
            pair<size_t> target = m_arg.next;
            ostringstream result;
            result << "JMP " << target.first << "." << target.second;
            return result.str();
        }
        case operation::BNG: {
            pair<size_t> target = m_arg.choice.second;
            ostringstream result;
            result << "BNG " << target.first << "." << target.second;
            return result.str();
        }
        case operation::TSP: {
            pair<size_t> target = m_arg.choice.second;
            ostringstream result;
            result << "TSP " << target.first << "." << target.second;
            return result.str();
        }
        default:
            cerr << "Unrecognized opcode '";
            printunichar(static_cast<int24_t>(m_op), cerr);
            cerr << '\'' << std::endl;
            exit(EXIT_FAILURE);
    }
}
