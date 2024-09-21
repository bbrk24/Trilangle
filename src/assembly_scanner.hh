#pragma once

#include <iosfwd>
#include <unordered_map>
#include "instruction.hh"

class assembly_scanner {
public:
    inline assembly_scanner(std::istream& program) : m_program(program), m_fragments(nullptr) {}

    inline ~assembly_scanner() noexcept {
        if (m_fragments == nullptr) {
            return;
        }
        for (std::vector<instruction>* frag : *m_fragments) {
            delete frag;
        }
        delete m_fragments;
    }

    NONNULL_PTR(const std::vector<std::vector<instruction>*>) get_fragments();
protected:
    void fake_location_to_real(std::pair<size_t, size_t>& p) const;
    static instruction::operation opcode_for_name(const std::string& name) noexcept;

    std::unordered_map<std::string, std::pair<size_t, size_t>> label_locations;
private:
    std::pair<size_t, size_t> get_current_location() const;
    void add_instruction(instruction&& i);

    std::istream& m_program;
    std::vector<std::vector<instruction>*>* m_fragments;
};
