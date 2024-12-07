#include "interpreter.hh"

#ifndef __EMSCRIPTEN__
extern "C" void send_thread_count(size_t) {}
#endif

template<>
void interpreter<program_walker>::split_thread(
    std::vector<thread<program_walker>>& new_threads,
    const thread<program_walker>& old_thread
) const {
    switch (old_thread.m_ip.dir) {
        case direction::east:
            new_threads.emplace_back(old_thread, direction::northeast);
            new_threads.emplace_back(old_thread, direction::southeast);
            break;
        case direction::west:
            new_threads.emplace_back(old_thread, direction::northwest);
            new_threads.emplace_back(old_thread, direction::southwest);
            break;
        default:
            unreachable("Only west- and east-moving threads can split");
    }
}

template<>
thread<program_walker> interpreter<program_walker>::join_threads(size_t first_index, size_t second_index) {
    thread<program_walker>& first_thread = m_threads[first_index];
    thread<program_walker>& second_thread = m_threads[second_index];

    direction new_dir;
    // They should either be facing both NW/SW or both NE/SE, never one going east and one going west
    assert((static_cast<char>(first_thread.m_ip.dir) & 0b011) == (static_cast<char>(second_thread.m_ip.dir) & 0b011));
    switch (first_thread.m_ip.dir) {
        case direction::northeast:
        case direction::southeast:
            new_dir = direction::east;
            break;
        case direction::northwest:
        case direction::southwest:
            new_dir = direction::west;
            break;
        default:
            unreachable("East- and west-going IPs cannot merge");
    }

    std::vector<int24_t> new_stack = join_threads_work(first_thread, second_thread);
    return thread<program_walker>(first_thread, new_dir, std::move(new_stack));
}
