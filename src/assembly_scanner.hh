#pragma once

#include <unordered_map>
#include "any_program_holder.hh"

class assembly_scanner : public any_program_holder<std::pair<size_t, size_t>> {
public:
    using IP = std::pair<size_t, size_t>;

    inline assembly_scanner(const std::string& program) : m_program(program), m_fragments(nullptr), m_slices(nullptr) {}

    inline ~assembly_scanner() noexcept {
        if (m_fragments == nullptr) {
            return;
        }
        delete m_slices;
        for (auto* frag : *m_fragments) {
            delete frag;
        }
        delete m_fragments;
    }

    NONNULL_PTR(const std::vector<NONNULL_PTR(std::vector<instruction>)>) get_fragments();

    void advance(IP& ip, std::function<bool()> go_left);

    inline std::string raw_at(const IP& ip) {
        DISCARD get_fragments();
        return slice(m_slices->at(ip.first)[ip.second]);
    }
    inline std::pair<size_t, size_t> get_coords(const IP& ip) const { return ip; }
    inline instruction at(const IP& ip) { return get_fragments()->at(ip.first)->at(ip.second); }
protected:
    void fake_location_to_real(IP& p) const;
    static instruction::operation opcode_for_name(const std::string& name) noexcept;

    std::unordered_map<std::string, IP> m_label_locations;
private:
    struct program_slice {
        size_t start;
        size_t end;

        constexpr size_t length() const noexcept { return end - start; }
    };

    IP get_current_location() const;
    void add_instruction(instruction&& i, const program_slice& s);

    inline std::string slice(const program_slice& s) const { return m_program.substr(s.start, s.length()); }

    std::string m_program;
    std::vector<NONNULL_PTR(std::vector<instruction>)>* m_fragments;
    std::vector<std::vector<program_slice>>* m_slices;
};
