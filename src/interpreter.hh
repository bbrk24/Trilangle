#pragma once

#include "input.hh"
#include "thread.hh"

class interpreter {
public:
    inline interpreter(NONNULL_PTR(program) p, flags f) noexcept : m_thread(p, f) {}

    void run();
private:
    thread m_thread;
};

extern "C" void send_thread_count(size_t thread_count);
