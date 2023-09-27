#include "instruction.hh"
#include "output.hh"

instruction::instruction(instruction_pointer ip, const program& program) noexcept : m_arg(), m_op(operation::NOP) {
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
