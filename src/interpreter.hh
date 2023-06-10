#pragma once

#include "input.hh"
#include "thread.hh"

class interpreter {
public:
    CONSTEXPR_ALLOC interpreter(NONNULL_PTR(const program) p, flags f) noexcept : m_threads{ thread(p, f) } {}

    void run();
private:
    // Combine two threads given their indices in m_threads, and return the result of joining them. May operate
    // destructively; the threads are assumed to not be used after this call.
    thread threadjoin(size_t first_index, size_t second_index);

    std::vector<thread> m_threads;
};