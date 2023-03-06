#include "interpreter.hh"
#include <algorithm>
#include <unordered_map>

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
        std::vector<size_t> removal_indices;
        std::vector<thread> pending_threads;
        std::unordered_map<std::pair<size_t, size_t>, size_t, pair_hash> waiting_coords;

        for (size_t i = 0; i < m_threads.size(); ++i) {
            auto& curr_thread = m_threads[i];

            if (curr_thread.get_status() != status::waiting) {
                curr_thread.tick();
            }

            switch (curr_thread.get_status()) {
                case status::splitting:
                    switch (curr_thread.get_ip().dir) {
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
                    auto location = waiting_coords.find(curr_thread.get_ip().coords);

                    if (location != waiting_coords.end()) {
                        size_t value = location->second;

                        removal_indices.push_back(value);
                        removal_indices.push_back(i);

                        direction new_dir;
                        switch (curr_thread.get_ip().dir) {
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

                        pending_threads.emplace_back(m_threads[value], new_dir);

                        waiting_coords.erase(location);
                    } else {
                        waiting_coords.insert({ curr_thread.get_ip().coords, i });
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
    }

    std::cout << std::flush;
}
