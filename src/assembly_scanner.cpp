#include "assembly_scanner.hh"
#include <cinttypes>
#include <cstring>
#include <iostream>
#include <set>
#include <sstream>

using std::cerr;
using std::endl;
using std::string;
using std::string_view;
using std::string_view_literals::operator""sv;

#define WHITESPACE " \n\r\t"

[[noreturn]] static void invalid_literal(const string& argument) {
    cerr << "Invalid format for literal: " << argument << endl;
    exit(EXIT_FAILURE);
}

const std::vector<instruction>& assembly_scanner::get_fragments() {
    if (m_fragments.has_value()) {
        return *m_fragments;
    }

    m_fragments.emplace(std::vector<instruction>());

    // We need to do two passes: one to resolve labels, and one to assign targets to jumps. During the first pass, the
    // fragments are actually constructed. However, jumps may not have valid targets yet, so we need some way to store
    // the label's name inside an IP. This code relies on the following assumption:
    static_assert(sizeof(NONNULL_PTR(const string)) <= sizeof(IP), "Cannot fit string pointer inside IP");
    // Using an ordered set over any other container so that references are not invalidated after insertion
    std::set<string> label_names;

    auto label_to_fake_location = [&](const string& name) -> IP {
        auto iter = label_names.find(name);
        if (iter == label_names.end()) {
            auto p = label_names.insert(name);
            iter = p.first;
        }
        NONNULL_PTR(const string) ptr = &*iter;
        return reinterpret_cast<uintptr_t>(ptr);
    };

    // First pass
    size_t line_end = 0;
    while (true) {
        size_t line_start = m_program.find_first_not_of('\n', line_end);
        if (line_start >= m_program.size()) {
            break;
        }
        line_end = m_program.find_first_of('\n', line_start);
        string_view curr_line = string_view(m_program).substr(line_start, line_end - line_start);

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
            curr_line.remove_suffix(curr_line.size() - i);
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
            curr_line.remove_suffix(curr_line.size() - i - 1);
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
            string label(curr_line.substr(0, i));

            [[maybe_unused]] auto _0 = label_names.insert(label);
            auto [_, inserted] = m_label_locations.insert({ label, m_fragments->size() });

            if (!inserted) {
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
        curr_line.remove_prefix(i);

        // Line should only be the opcode and, if there is one, the argument
        if (curr_line.size() < 3) {
            cerr << "Instruction too short: " << curr_line << endl;
            exit(EXIT_FAILURE);
        }
        if (curr_line.size() > 3 && !strchr(WHITESPACE, curr_line[3])) {
            cerr << "Instruction too long: " << curr_line << endl;
            exit(EXIT_FAILURE);
        }

        string_view instruction_name(curr_line.data(), 3);
        instruction::operation opcode = assembly_scanner::opcode_for_name(instruction_name);
        switch (opcode) {
            case instruction::operation::JMP: {
                size_t label_start = curr_line.find_first_not_of(WHITESPACE, 3);
                instruction::argument arg;
                string label(curr_line.substr(label_start));
                arg.next = { SIZE_C(0), label_to_fake_location(label) };
                m_fragments->push_back({ opcode, arg });
                m_slices.push_back(curr_line);
                break;
            }
            case instruction::operation::BNG:
            case instruction::operation::TSP: {
                size_t label_start = curr_line.find_first_not_of(WHITESPACE, 3);
                instruction::argument arg;
                string label(curr_line.substr(label_start));
                arg.choice = { { SIZE_C(0), m_fragments->size() + 1 }, { SIZE_C(0), label_to_fake_location(label) } };
                m_fragments->push_back({ opcode, arg });
                m_slices.push_back(curr_line);
                break;
            }
            case instruction::operation::PSI:
            case instruction::operation::PSC: {
                size_t arg_start = curr_line.find_first_not_of(WHITESPACE, 3);
                if (arg_start == string::npos) {
                    cerr << "Missing argument for push instruction" << endl;
                    exit(EXIT_FAILURE);
                }

                string argument(curr_line.substr(arg_start));
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
                    arg_value = parse_unichar([&]() { return argument[i++]; });
                    if (arg_value < INT24_C(0) || i != argument.size() - 1) {
                        invalid_literal(argument);
                    }
                } else if (argument[0] == '0' && argument[1] == 'x') {
                    char* last = nullptr;
                    unsigned long ul = strtoul(argument.c_str(), &last, 16);
                    if (*last != '\0' || ul > 0x1f'ffffUL) {
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

                if (opcode == instruction::operation::PSI) {
                    // PSI expects to be given the digit, not the actual value
                    auto p = arg_value.add_with_overflow('0');
                    assert(!p.first);
                    arg_value = p.second;
                }

                instruction::argument arg;
                arg.number = arg_value;
                m_fragments->push_back({ opcode, arg });
                m_slices.push_back(curr_line);
                break;
            }
            default:
                m_fragments->push_back({ opcode, instruction::argument() });
                m_slices.push_back(curr_line);
                break;
        }
    }

    // Second pass
    for (auto& instr : *m_fragments) {
        if (instr.m_op == instruction::operation::JMP) {
            fake_location_to_real(instr.m_arg.next);
        } else if (instr.m_op == instruction::operation::TSP || instr.m_op == instruction::operation::BNG) {
            fake_location_to_real(instr.m_arg.choice.second);
        }
    }

    return *m_fragments;
}

void assembly_scanner::advance(IP& ip, std::function<bool()> go_left) {
    instruction i = at(ip);

    if (i.get_op() == instruction::operation::JMP) {
        ip = i.get_arg().next.second;
        return;
    }

    const auto* to_left = i.second_if_branch();
    if (to_left != nullptr && go_left()) {
        ip = to_left->second;
        return;
    }

    ip++;
}

void assembly_scanner::fake_location_to_real(std::pair<size_t, size_t>& p) const {
    uintptr_t reconstructed = static_cast<uintptr_t>(p.second);
    auto ptr = reinterpret_cast<NONNULL_PTR(const string)>(reconstructed);
    const string& str = *ptr;
    auto loc = m_label_locations.find(str);
    if (loc == m_label_locations.end()) {
        cerr << "Undeclared label '" << str << "'" << endl;
        exit(EXIT_FAILURE);
    }
    p = { SIZE_C(0), loc->second };
}

#define DESTRINGIFY_NAME(op) \
    if (name == #op##sv) \
    return instruction::operation::op

instruction::operation assembly_scanner::opcode_for_name(const string_view& name) noexcept {
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
