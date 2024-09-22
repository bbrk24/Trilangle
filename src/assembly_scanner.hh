#pragma once

#include <iosfwd>
#include <unordered_map>
#include "any_program_holder.hh"

class assembly_scanner : public any_program_holder<std::pair<size_t, size_t> > {
public:
    using IP = std::pair<size_t, size_t>;

    inline assembly_scanner(std::istream* program) : m_program(program), m_fragments(nullptr) {}

    inline ~assembly_scanner() noexcept {
        if (m_fragments == nullptr) {
            return;
        }
        for (auto* frag : *m_fragments) {
            delete frag;
        }
        delete m_fragments;
    }

    NONNULL_PTR(const std::vector<NONNULL_PTR(std::vector<instruction>)>) get_fragments();

    void advance(IP& ip, std::function<bool()> go_left) const;
    std::string raw_at(const IP& ip) const;

    inline std::pair<size_t, size_t> get_coords(const IP& ip) const { return ip; }
    inline instruction at(const IP& ip) const { return m_fragments->at(ip.first)->at(ip.second); }
protected:
    void fake_location_to_real(IP& p) const;
    static instruction::operation opcode_for_name(const std::string& name) noexcept;

    std::unordered_map<std::string, IP> label_locations;
private:
    IP get_current_location() const;
    void add_instruction(instruction&& i);

    std::istream* m_program;
    std::vector<NONNULL_PTR(std::vector<instruction>)>* m_fragments;
};
