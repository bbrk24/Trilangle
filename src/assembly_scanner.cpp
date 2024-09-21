#include "assembly_scanner.hh"
#include <cinttypes>
#include <iostream>
#include <set>
#include <unordered_map>

using std::cerr;
using std::endl;
using std::pair;
using std::string;

#define WHITESPACE " \n\r\t"

[[noreturn]] static void invalid_literal(const string& argument) {
    cerr << "Invalid format for literal: " << argument << endl;
    exit(EXIT_FAILURE);
}

// Disable warnings relating to shifting by too much
#ifdef __clang__
#pragma clang diagnostic ignored "-Wshift-count-overflow"
#elif REALLY_MSVC
#pragma warning(disable: 4293)
#else
#pragma GCC diagnostic ignored "-Wshift-count-overflow"
#endif

NONNULL_PTR(const std::vector<std::vector<instruction>*>) assembly_scanner::get_fragments() {
    if (m_fragments != nullptr) {
        return m_fragments;
    }

    m_fragments = new std::vector<std::vector<instruction>*>();

    // We need to do two passes: one to resolve labels, and one to assign targets to jumps. During the first pass, the
    // fragments are actually constructed. However, jumps may not have valid targets yet, so we need some way to store
    // the label's name inside a pair<size_t, size_t>. This code relies on the following assumption:
    static_assert(
        sizeof(NONNULL_PTR(const string)) <= 2 * sizeof(size_t), "Cannot fit string pointer inside pair of size_t"
    );
    // Using an ordered set over any other container so that references are not invalidated after insertion
    std::set<string> label_names;

    auto label_to_fake_location = [&](const string& name) -> pair<size_t, size_t> {
        auto iter = label_names.find(name);
        if (iter == label_names.end()) {
            auto p = label_names.insert(name);
            iter = p.first;
        }
        NONNULL_PTR(const string) ptr = &*iter;
        size_t bottom_half = reinterpret_cast<uintptr_t>(ptr) & SIZE_MAX;
        size_t top_half = reinterpret_cast<uintptr_t>(ptr) >> (8 * sizeof(size_t));
        return { top_half, bottom_half };
    };

    // First pass
    for (string curr_line; std::getline(m_program, curr_line);) {
        // Unquoted semicolons are comments. Remove them.
        size_t i;
        for (i = 0; i < curr_line.size(); ++i) {
            if (curr_line[i] != ';') {
                continue;
            }
            if (i == 0 || curr_line[i - 1] != '\'') {
                break;
            }
        }
        if (i < curr_line.size()) {
            curr_line.erase(i);
        }
        // If the line is only a comment, move on
        if (curr_line.empty()) {
            continue;
        }
        // Remove trailing whitespace. If there's only whitespace, skip this line
        i = curr_line.find_last_not_of(WHITESPACE);
        if (i == string::npos) {
            continue;
        } else {
            curr_line.erase(i + 1);
        }

        // Look for labels (non-whitespace in the first column)
        i = curr_line.find_first_not_of(WHITESPACE ":");
        assert(i != string::npos);
        if (i == 0) {
            // Label, find end
            i = curr_line.find_first_of(WHITESPACE ":");
            if (i == string::npos) {
                i = curr_line.size();
            }
            string label = curr_line.substr(0, i);

            DISCARD label_names.insert(label);
            auto p = label_locations.insert({ label, get_current_location() });

            if (!p.second) {
                cerr << "Label '" << label << "' appears twice" << endl;
                exit(EXIT_FAILURE);
            }

            // Set i to the first non-whitespace character after the label
            i = curr_line.find_first_not_of(WHITESPACE ":", i + 1);
            if (i == string::npos) {
                // Line was only a label
                continue;
            }
        }

        // Remove leading whitespace, and label if there is one
        curr_line.erase(0, i);

        // Line should only be the opcode and, if there is one, the argument
        if (curr_line.size() < 3) {
            cerr << "Instruction too short: " << curr_line << endl;
            exit(EXIT_FAILURE);
        }

        string instruction_name = curr_line.substr(0, 3);
        instruction::operation opcode = assembly_scanner::opcode_for_name(instruction_name);
        switch (opcode) {
            case instruction::operation::JMP: {
                size_t label_start = curr_line.find_first_not_of(WHITESPACE, 3);
                instruction::argument arg;
                string label = curr_line.substr(label_start);
                arg.next = label_to_fake_location(label);
                add_instruction({ opcode, arg });
                break;
            }
            case instruction::operation::BNG:
            case instruction::operation::TSP: {
                size_t label_start = curr_line.find_first_not_of(WHITESPACE, 3);
                instruction::argument arg;
                string label = curr_line.substr(label_start);
                arg.choice = { { get_current_location().first + 1, SIZE_C(0) }, label_to_fake_location(label) };
                add_instruction({ opcode, arg });
                break;
            }
            case instruction::operation::PSI:
            case instruction::operation::PSC: {
                size_t arg_start = curr_line.find_first_not_of(WHITESPACE, 3);
                if (arg_start == string::npos) {
                    cerr << "Missing argument for push instruction" << endl;
                    exit(EXIT_FAILURE);
                }

                string argument = curr_line.substr(arg_start);
                int24_t arg_value;
                // Should be in one of three formats:
                // - 'c' (single UTF-8 character)
                // - 0xff (arbitrary length hex number)
                // - #9 (single decimal digit)
                if (argument[0] == '\'' && argument.back() == '\'') {
                    if (argument.size() < 3 || argument.size() > 6) {
                        // One UTF-8 character is 1 to 4 bytes
                        invalid_literal(argument);
                    }
                    i = 1;
                    arg_value = parse_unichar([&]() { return curr_line[i++]; });
                    if (arg_value < INT24_C(0) || i != argument.size() - 1) {
                        invalid_literal(argument);
                    }
                } else if (argument[0] == '0' && argument[1] == 'x') {
                    char* last = nullptr;
                    unsigned long ul = strtoul(argument.c_str(), &last, 16);
                    if (*last != '\0') {
                        invalid_literal(argument);
                    }
                    arg_value = static_cast<int24_t>(ul);
                } else if (argument[0] == '#' && argument.size() == 2) {
                    arg_value = static_cast<int24_t>(argument[1] - '0');
                    if (arg_value < INT24_C(0) || arg_value > INT24_C(9)) {
                        invalid_literal(argument);
                    }
                } else {
                    invalid_literal(argument);
                }

                instruction::argument arg;
                arg.number = arg_value;
                add_instruction({ opcode, arg });
                break;
            }
            default:
                add_instruction({ opcode, instruction::argument() });
                break;
        }
    }

    // Second pass
    for (auto* fragment : *m_fragments) {
        for (auto& instr : *fragment) {
            if (instr.m_op == instruction::operation::JMP) {
                fake_location_to_real(instr.m_arg.next);
            } else if (instr.m_op == instruction::operation::TSP || instr.m_op == instruction::operation::BNG) {
                fake_location_to_real(instr.m_arg.choice.second);
            }
        }
    }

    return m_fragments;
}

pair<size_t, size_t> assembly_scanner::get_current_location() const {
    size_t first = this->m_fragments->size();
    size_t second = 0;
    if (first > 0) {
        --first;
        const auto* ptr = this->m_fragments->at(first);
        if (ptr == nullptr) {
            second = ptr->size();
            if (second > 0) {
                --second;
            }
        }
    }
    return { first, second };
}

void assembly_scanner::add_instruction(instruction&& i) {
    assert(m_fragments != nullptr);
    if (m_fragments->empty()) {
        m_fragments->push_back(new std::vector<instruction>{ std::move(i) });
    } else {
        auto last = m_fragments->back();
        if (last == nullptr) {
            size_t idx = m_fragments->size() - 1;
            m_fragments->at(idx) = new std::vector<instruction>{ std::move(i) };
        } else {
            if (last->back().is_exit() || last->back().first_if_branch()
                || last->back().m_op == instruction::operation::JMP) {
                m_fragments->push_back(new std::vector<instruction>{ std::move(i) });
            } else {
                last->push_back(std::forward<instruction&&>(i));
            }
        }
    }
}

void assembly_scanner::fake_location_to_real(pair<size_t, size_t>& p) const {
    uintptr_t reconstructed =
        (static_cast<uintptr_t>(p.first) << (8 * sizeof(size_t))) | static_cast<uintptr_t>(p.second);
    auto ptr = reinterpret_cast<NONNULL_PTR(const string)>(reconstructed);
    p = label_locations.find(*ptr)->second;
}

#define DESTRINGIFY_NAME(op) \
    if (name == #op) \
    return instruction::operation::op

instruction::operation assembly_scanner::opcode_for_name(const std::string& name) noexcept {
    DESTRINGIFY_NAME(BNG);
    DESTRINGIFY_NAME(JMP);
    DESTRINGIFY_NAME(TKL);
    DESTRINGIFY_NAME(TSP);
    DESTRINGIFY_NAME(TJN);
    DESTRINGIFY_NAME(TKL);
    DESTRINGIFY_NAME(NOP);
    DESTRINGIFY_NAME(ADD);
    DESTRINGIFY_NAME(SUB);
    DESTRINGIFY_NAME(MUL);
    DESTRINGIFY_NAME(DIV);
    DESTRINGIFY_NAME(UDV);
    DESTRINGIFY_NAME(MOD);
    DESTRINGIFY_NAME(PSI);
    DESTRINGIFY_NAME(PSC);
    DESTRINGIFY_NAME(POP);
    DESTRINGIFY_NAME(EXT);
    DESTRINGIFY_NAME(INC);
    DESTRINGIFY_NAME(DEC);
    DESTRINGIFY_NAME(AND);
    DESTRINGIFY_NAME(IOR);
    DESTRINGIFY_NAME(XOR);
    DESTRINGIFY_NAME(NOT);
    DESTRINGIFY_NAME(GTC);
    DESTRINGIFY_NAME(PTC);
    DESTRINGIFY_NAME(GTI);
    DESTRINGIFY_NAME(PTI);
    DESTRINGIFY_NAME(PTU);
    DESTRINGIFY_NAME(IDX);
    DESTRINGIFY_NAME(DUP);
    DESTRINGIFY_NAME(DP2);
    DESTRINGIFY_NAME(RND);
    DESTRINGIFY_NAME(EXP);
    DESTRINGIFY_NAME(SWP);
    DESTRINGIFY_NAME(GTM);
    DESTRINGIFY_NAME(GDT);

    cerr << "Unrecognized opcode '" << name << '\'' << endl;
    exit(EXIT_FAILURE);
}
