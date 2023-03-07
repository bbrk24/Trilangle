#pragma once

#include "input.hh"
#include "thread.hh"

class interpreter {
public:
    CONSTEXPR_ALLOC interpreter(NONNULL_PTR(const program) p, flags f) noexcept : m_threads{ thread(p, f) } {}

    void run();

    static void stop_all() noexcept;
private:
    std::vector<thread> m_threads;
};
