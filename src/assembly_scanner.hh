#pragma once

#include <optional>
#include <string_view>
#include <unordered_map>
#include "any_program_holder.hh"

class assembly_scanner : public any_program_holder<std::pair<size_t, size_t>> {
public:
    using IP = std::pair<size_t, size_t>;

    CONSTEXPR_ALLOC assembly_scanner(const std::string& program) :
        m_program(program), m_fragments(std::nullopt), m_slices{} {}

    const std::vector<std::optional<std::vector<instruction>>>& get_fragments();

    void advance(IP& ip, std::function<bool()> go_left);

    inline std::string raw_at(const IP& ip) {
        [[maybe_unused]] auto _ = get_fragments();
        return std::string(m_slices[ip.first][ip.second]);
    }
    inline std::pair<size_t, size_t> get_coords(const IP& ip) const { return ip; }
    inline instruction at(const IP& ip) { return get_fragments()[ip.first]->at(ip.second); }
protected:
    void fake_location_to_real(IP& p) const;
    static instruction::operation opcode_for_name(const std::string_view& name) noexcept;

    std::unordered_map<std::string, IP> m_label_locations;
private:
    IP get_current_location() const;
    void add_instruction(instruction&& i, const std::string_view& s);

    std::string m_program;
    std::optional<std::vector<std::optional<std::vector<instruction>>>> m_fragments;
    std::vector<std::vector<std::string_view>> m_slices;
};
