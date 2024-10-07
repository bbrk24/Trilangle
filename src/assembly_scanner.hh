#pragma once

#include <optional>
#include <string_view>
#include <unordered_map>
#include "any_program_holder.hh"

class assembly_scanner : public any_program_holder<size_t> {
public:
    using IP = size_t;

    inline assembly_scanner(const std::string& program) :
        m_label_locations(), m_program(program), m_fragments(std::nullopt), m_slices{} {}

    const std::vector<instruction>& get_fragments();

    void advance(IP& ip, std::function<bool()> go_left);

    inline std::string raw_at(const IP& ip) {
        [[maybe_unused]] auto _ = get_fragments();
        return std::string(m_slices[ip]);
    }
    inline std::pair<size_t, size_t> get_coords(const IP& ip) const { return {SIZE_C(0), ip}; }
    inline instruction at(const IP& ip) { return get_fragments()[ip]; }
private:
    void fake_location_to_real(std::pair<size_t, size_t>& p) const;
    static instruction::operation opcode_for_name(const std::string_view& name) noexcept;

    std::unordered_map<std::string, IP> m_label_locations;
    std::string m_program;
    std::optional<std::vector<instruction>> m_fragments;
    std::vector<std::string_view> m_slices;
};
