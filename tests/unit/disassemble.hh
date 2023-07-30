#pragma once

#include <disassembler.hh>
#include "test-framework/test_framework.hh"

namespace {
class test_disassemble : public disassembler {
public:
    inline test_disassemble(NONNULL_PTR(const program) p, flags f) : disassembler(p, f) {}

    inline NONNULL_PTR(const std::vector<std::vector<instruction>*>) get_fragments() noexcept {
        if (m_fragments == nullptr) {
            build_state();
        }
        return m_fragments;
    }
};
}  // namespace

testgroup (disassemble) {
    testcase (exit_immediately) {
        program p("@");
        flags f;
        f.disassemble = true;

        test_disassemble td(&p, f);

        auto* fragments = td.get_fragments();
        test_assert(fragments->size() == 1, "Program without branches or threads should have 1 fragment");
        auto* fragment = fragments->at(0);
        test_assert(fragment->size() == 1, "Program of 1 character should be 1 instruction");
        test_assert(fragment->back().is_exit(), "Program must end in EXT");
    }
    , testcase (infinite_loop) {
        program p(".");
        flags f;
        f.disassemble = true;

        test_disassemble td(&p, f);

        auto* fragments = td.get_fragments();
        test_assert(fragments->size() == 1, "Program without branches or threads should have 1 fragment");
        auto* fragment = fragments->at(0);
        // 1 for the instruction itself and one for the JMP
        // In the general case, this has to be two (e.g. GTC). I'm just using NOP here because it's detectable.
        test_assert(fragment->size() == 2);
        test_assert(fragment->front().is_nop());
        auto& last_instruction = fragment->back();
        test_assert(
            !last_instruction.is_exit() && !last_instruction.is_join() && !last_instruction.is_nop(),
            "Instruction should be JMP, not EXT, TJN, or NOP"
        );
    }
    , testcase (branch) {
        program p(R"(
           <
          . ?
         . > .
        . @ . .
        )");
        flags f;
        f.disassemble = true;

        test_disassemble td(&p, f);

        auto* fragments = td.get_fragments();
        test_assert(fragments->size() == 3, "Program with one branch should have 3 fragments");
        test_assert(fragments->at(0)->back().first_if_branch(), "First fragment should end in branch");
        test_assert(fragments->at(1)->back().is_exit(), "Second fragment should end in EXT");
        test_assert(fragments->at(2)->back().is_exit(), "Third fragment should end in EXT");
    }
};
