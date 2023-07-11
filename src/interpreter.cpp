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

struct pair_hash {
    constexpr size_t operator()(const std::pair<size_t, size_t>& p) const noexcept { return p.first ^ (p.second << 4); }
};

void interpreter::run() {
    thread::thread_count = 0UL;

    // Begin the execution loop.
    while (true) {
        vector<size_t> removal_indices;
        vector<thread> pending_threads;
        std::unordered_map<std::pair<size_t, size_t>, size_t, pair_hash> waiting_coords;

        for (size_t i = 0; i < m_threads.size(); ++i) {
            thread& curr_thread = m_threads[i];

            curr_thread.tick();

            switch (curr_thread.m_status) {
                case status::splitting:
                    switch (curr_thread.m_ip.dir) {
                        case direction::east:
                            pending_threads.emplace_back(curr_thread, direction::northeast);
                            pending_threads.emplace_back(curr_thread, direction::southeast);
                            break;
                        case direction::west:
                            pending_threads.emplace_back(curr_thread, direction::northwest);
                            pending_threads.emplace_back(curr_thread, direction::southwest);
                            break;
                        default:
                            unreachable("Only west- and east-moving threads can split");
                    }
                    FALLTHROUGH
                case status::terminated:
                    removal_indices.push_back(i);
                    break;
                case status::waiting: {
                    auto location = waiting_coords.find(curr_thread.m_ip.coords);

                    if (location != waiting_coords.end()) {
                        size_t value = location->second;

                        pending_threads.push_back(threadjoin(value, i));

                        removal_indices.push_back(value);
                        removal_indices.push_back(i);

                        waiting_coords.erase(location);
                    } else {
                        waiting_coords.insert({ curr_thread.m_ip.coords, i });
                    }

                    break;
                }
                case status::idle:
                    break;
                case status::active:
                    curr_thread.advance();
                    break;
            }
        }

        // Sort the removal indices descending, so that we can erase them in a single loop
        std::sort(removal_indices.begin(), removal_indices.end(), std::greater<size_t>{});

        for (size_t i : removal_indices) {
            m_threads.erase(m_threads.cbegin() + static_cast<ptrdiff_t>(i));
        }

        // Move the pending threads into the thread vector
        m_threads.insert(
            m_threads.cend(),
            std::make_move_iterator(pending_threads.begin()),
            std::make_move_iterator(pending_threads.end())
        );

        if (m_threads.empty()) {
            std::cout << std::flush;
            cerr << std::flush;
            return;
        }
    }
}

thread interpreter::threadjoin(size_t first_index, size_t second_index) {
    thread& first_thread = m_threads[first_index];
    thread& second_thread = m_threads[second_index];

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

    int24_t first_stack_amount = first_thread.m_stack.back();
    first_thread.m_stack.pop_back();

    if (first_thread.m_flags.warnings && first_stack_amount > static_cast<int24_t>(first_thread.m_stack.size()))
        UNLIKELY {
        cerr << "Warning: Attempt to move " << first_stack_amount << " values out of thread with stack size "
             << first_thread.m_stack.size() << '\n';
    }

    int24_t second_stack_amount = second_thread.m_stack.back();
    second_thread.m_stack.pop_back();

    bool whole_second_stack = second_stack_amount < INT24_C(0);

    vector<int24_t> new_stack;
    if (first_stack_amount < INT24_C(0)) {
        new_stack = std::move(first_thread.m_stack);
    } else {
        // The iterator-based constructor copies elementwise and leaves no extra space. To prevent the unneeded
        // reallocation, reserve the needed size before copying out of the first stack.
        new_stack.reserve(
            static_cast<size_t>(first_stack_amount)
            + (whole_second_stack ? second_thread.m_stack.size() : static_cast<size_t>(second_stack_amount))
        );
        new_stack.insert(
            new_stack.cbegin(),
            first_thread.m_stack.end() - static_cast<ptrdiff_t>(first_stack_amount),
            first_thread.m_stack.end()
        );
    }

    if (second_thread.m_flags.warnings && second_stack_amount > static_cast<int24_t>(second_thread.m_stack.size()))
        UNLIKELY {
        cerr << "Warning: Attempt to move " << second_stack_amount << " values out of thread with stack size "
             << second_thread.m_stack.size() << '\n';
    }

    new_stack.insert(
        new_stack.cend(),
        whole_second_stack ? second_thread.m_stack.begin()
                           : (second_thread.m_stack.end() - static_cast<ptrdiff_t>(second_stack_amount)),
        second_thread.m_stack.end()
    );

    return thread(first_thread, new_dir, std::move(new_stack));
}
