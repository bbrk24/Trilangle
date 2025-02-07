#include "interpreter.hh"
#include <algorithm>
#include <cstddef>
#include <functional>
#include <iostream>
#include <iterator>
#include <unordered_map>

using std::cerr;
using std::vector;

using status = thread::status;

#ifndef __EMSCRIPTEN__
extern "C" void send_thread_count(size_t) {}
#endif

struct pair_hash {
    constexpr size_t operator()(const std::pair<size_t, size_t>& p) const noexcept { return p.first ^ (p.second << 4); }
};

void interpreter::run() {
    thread::thread_count = 0UL;

    // Begin the execution loop.
    while (true) {
        send_thread_count(SIZE_C(1));

        m_thread.tick();

        switch (m_thread.m_status) {
            case status::terminated:
                return;
            case status::idle:
                break;
            case status::active:
                m_thread.advance();
                break;
        }
    }
}
