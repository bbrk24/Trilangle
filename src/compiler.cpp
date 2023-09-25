// This file is not in the compiler folder because the MSVC build script requires this folder to contain all the .cpp
// files.
#include "compiler.hh"
#include <iostream>
#include "compiler/strings.hh"

void compiler::write_state(std::ostream& os) {
    if (m_fragments == nullptr) {
        build_state();
    }

    os << header;

    for (size_t i = 0; i < m_fragments->size(); ++i) {
        const std::vector<instruction>& frag = *m_fragments->at(i);
        for (size_t j = 0; j < frag.size(); ++j) {
            os << "\nlbl" << i << '_' << j << ": ";
            get_c_code(frag[j], os);
        }
    }

    os << footer << std::flush;
}

void compiler::get_c_code(const instruction& i, std::ostream& os) {
    using op = instruction::operation;

    switch (i.m_op) {
        case op::BNG: {
            os << "if (lws_top(stack) < 0) goto lbl";
            const auto& choice = i.m_arg.choice;
            const auto &dest1 = choice.first, &dest2 = choice.second;
            os << dest2.first << '_' << dest2.second << "; goto lbl" << dest1.first << '_' << dest1.second << ';';
            return;
        }
        case op::JMP: {
            os << "goto lbl";
            const auto& dest = i.m_arg.next;
            os << dest.first << '_' << dest.second << ';';
            return;
        }
        case op::TKL:
        case op::EXT:
            os << "lws_deinit(stack); return 0;";
            return;
        case op::TJN:
        case op::TSP:
            std::cerr << "Threading is not supported for compiled programs." << std::endl;
            exit(EXIT_FAILURE);
        case op::NOP:
            break;
        case op::ADD:
            os << "lws_push(stack, lws_pop(stack) + lws_pop(stack));";
            return;
        case op::SUB:
            // The calls need to be well-ordered
            os << "{ int32_t tmp = lws_pop(stack); lws_push(stack, lws_pop(stack) - tmp); }";
            return;
        case op::MUL:
            os << "lws_push(stack, lws_pop(stack) * lws_pop(stack));";
            return;
        case op::DIV:
            os << "{ int32_t tmp = lws_pop(stack); lws_push(stack, lws_pop(stack) / tmp); }";
            return;
        case op::MOD:
            os << "{ int32_t tmp = lws_pop(stack); lws_push(stack, lws_pop(stack) % tmp); }";
            return;
        case op::PSC: {
            os << "lws_push(stack,";
            const auto value = i.m_arg.number;
            os << static_cast<int32_t>(value) << ");";
            return;
        }
        case op::PSI: {
            os << "lws_push(stack,";
            const auto value = i.m_arg.number;
            os << static_cast<int32_t>(value - (int24_t)'0') << ");";
            return;
        }
        case op::POP:
            os << "(void)lws_pop(stack);";
            return;
        case op::INC:
            os << "lws_push(stack, (lws_pop(stack) + 1) << 8 >> 8);";
            return;
        case op::DEC:
            os << "lws_push(stack, (lws_pop(stack) - 1) << 8 >> 8);";
            return;
        case op::AND:
            os << "lws_push(stack, lws_pop(stack) & lws_pop(stack));";
            return;
        case op::IOR:
            os << "lws_push(stack, lws_pop(stack) | lws_pop(stack));";
            return;
        case op::XOR:
            os << "lws_push(stack, lws_pop(stack) ^ lws_pop(stack));";
            return;
        case op::NOT:
            os << "lws_push(stack, ~lws_pop(stack));";
            return;
        case op::GTC:
            os << "{ wint_t temp = getwchar(); lws_push(stack, temp == WEOF ? -1 : temp); }";
            return;
        case op::PTC:
            os << "putwchar(lws_top(stack));";
            return;
        case op::GTI:
            os << "{"
                  "int32_t i = -1;"
                  "while (!(feof(stdin) || scanf(\"%\" SCNi32, &i))) (void)getchar();"
                  "lws_push(stack, i);"
                  "}";
            return;
        case op::PTI:
            os << R"( printf("%" PRId32 "\n", lws_top(stack)); )";
            return;
        case op::IDX:
            os << "lws_push(stack, lws_index(stack, lws_pop(stack)));";
            return;
        case op::DUP:
            os << "lws_push(stack, lws_top(stack));";
            return;
        case op::DP2: {
            const char* str = "lws_push(stack, lws_index(stack, 1));";
            os << str << str;
            return;
        }
        case op::RND:
        case op::GDT:
        case op::GTM:
            std::cerr << "Nondeterministic instructions are not supported for compiled programs." << std::endl;
            exit(EXIT_FAILURE);
        case op::EXP:
            os << "lws_push(stack, 1 << lws_pop(stack));";
            return;
        case op::SWP:
            os << "{"
                  "int32_t temp1 = lws_pop(stack);"
                  "int32_t temp2 = lws_pop(stack);"
                  "lws_push(stack, temp1);"
                  "lws_push(stack, temp2);"
                  "}";
            return;
        default:
            std::cerr << "Unknown opcode." << std::endl;
            exit(EXIT_FAILURE);
    }
}
