#pragma once

#include <functional>
#include "instruction.hh"

template<typename IP>
class any_program_holder {
public:
    virtual void advance(IP& ip, std::function<bool()> go_left) const = 0;
    virtual instruction at(const IP& ip) const = 0;
    virtual std::string raw_at(const IP& ip) const = 0;
    virtual std::pair<size_t, size_t> get_coords(const IP& ip) const = 0;
};
