#include "interpreter.hh"
#include <algorithm>
#include <iostream>
#include <unordered_map>

// When targeting the web, the page can't re-render until the C++ code finishes.
// emscripten_sleep is asynchronous (so it allows a re-render), but it looks synchronous from the C side.
// Insert a call to emscripten_sleep(0) whenever the page needs a chance to redraw, such as after a print statement.
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#else
#define emscripten_sleep(x) ((void)0)
#endif

using std::cerr;
using std::vector;

using status = thread::status;

static bool should_run;

struct pair_hash {
    constexpr size_t operator()(const std::pair<size_t, size_t>& p) const noexcept { return p.first ^ (p.second << 4); }
};

void interpreter::stop_all() noexcept {
    should_run = false;
}

void interpreter::run() {
    should_run = true;
    thread::thread_count = 0UL;

    // Begin the execution loop.
    while (should_run) {
        vector<size_t> removal_indices;
        vector<thread> pending_threads;
        std::unordered_map<std::pair<size_t, size_t>, size_t, pair_hash> waiting_coords;
        bool should_sleep = false;

        for (size_t i = 0; i < m_threads.size(); ++i) {
            thread& curr_thread = m_threads[i];

            if (curr_thread.m_status != status::waiting) {
                curr_thread.tick(should_sleep);
            }

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
                        thread& other_thread = m_threads[value];

                        removal_indices.push_back(value);
                        removal_indices.push_back(i);

                        direction new_dir;
                        switch (curr_thread.m_ip.dir) {
                            case direction::northeast:
                                FALLTHROUGH
                            case direction::southeast:
                                new_dir = direction::east;
                                break;
                            case direction::northwest:
                                FALLTHROUGH
                            case direction::southwest:
                                new_dir = direction::west;
                                break;
                            default:
                                unreachable("East- and west-going IPs cannot merge");
                        }

                        int24_t other_stack_amount = other_thread.m_stack.back();
                        other_thread.m_stack.pop_back();

                        if (curr_thread.m_flags.warnings
                            && other_stack_amount > static_cast<int24_t>(other_thread.m_stack.size())) UNLIKELY {
                            cerr << "Warning: Attempt to move " << other_stack_amount
                                 << " values out of thread with stack size " << other_thread.m_stack.size() << '\n';
                        }

                        vector<int24_t> new_stack(
                            other_stack_amount < INT24_C(0)
                                ? other_thread.m_stack.begin()
                                : (other_thread.m_stack.end() - static_cast<ptrdiff_t>(other_stack_amount)),
                            other_thread.m_stack.end()
                        );

                        int24_t this_stack_amount = curr_thread.m_stack.back();
                        curr_thread.m_stack.pop_back();

                        if (curr_thread.m_flags.warnings
                            && this_stack_amount > static_cast<int24_t>(curr_thread.m_stack.size())) UNLIKELY {
                            cerr << "Warning: Attempt to move " << this_stack_amount
                                 << " values out of thread with stack size " << curr_thread.m_stack.size() << '\n';
                        }

                        new_stack.insert(
                            new_stack.end(),
                            this_stack_amount < INT24_C(0)
                                ? curr_thread.m_stack.begin()
                                : (curr_thread.m_stack.end() - static_cast<ptrdiff_t>(this_stack_amount)),
                            curr_thread.m_stack.end()
                        );

                        pending_threads.emplace_back(m_threads[value], new_dir, std::move(new_stack));

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

        std::sort(removal_indices.begin(), removal_indices.end(), [](size_t x, size_t y) NOEXCEPT_T { return x > y; });

        for (size_t i : removal_indices) {
            m_threads.erase(m_threads.cbegin() + static_cast<ptrdiff_t>(i));
        }

        m_threads.insert(m_threads.cend(), pending_threads.begin(), pending_threads.end());

        if (m_threads.empty()) {
            break;
        }

        if (should_sleep) {
            emscripten_sleep(0);
        }
    }

    std::cout << std::flush;
    cerr << std::flush;
}
