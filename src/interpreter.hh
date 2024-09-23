#pragma once

#include <cstddef>
#include <iterator>
#include "assembly_scanner.hh"
#include "thread.hh"

struct pair_hash {
    constexpr size_t operator()(const std::pair<size_t, size_t>& p) const noexcept { return p.first ^ (p.second << 4); }
};

extern "C" void send_thread_count(size_t tc);

template<class ProgramHolder>
class interpreter {
public:
    inline interpreter(ProgramHolder& p, flags f) noexcept :
        m_program_holder(p), m_threads{ thread<ProgramHolder>(&p, f) } {}

    inline void run() {
        using status = typename thread<ProgramHolder>::status;

        thread_count = 0UL;

        // Begin the execution loop.
        while (true) {
            std::vector<size_t> removal_indices;
            std::vector<thread<ProgramHolder>> pending_threads;
            std::unordered_map<std::pair<size_t, size_t>, size_t, pair_hash> waiting_coords;

            send_thread_count(m_threads.size());

            for (size_t i = 0; i < m_threads.size(); ++i) {
                thread<ProgramHolder>& curr_thread = m_threads[i];

                curr_thread.tick();

                switch (curr_thread.m_status) {
                    case status::splitting:
                        split_thread(pending_threads, curr_thread);
                        FALLTHROUGH
                    case status::terminated:
                        removal_indices.push_back(i);
                        break;
                    case status::waiting: {
                        std::pair<size_t, size_t> coords = m_program_holder.get_coords(curr_thread.m_ip);
                        auto location = waiting_coords.find(coords);

                        if (location != waiting_coords.end()) {
                            size_t value = location->second;

                            pending_threads.push_back(join_threads(value, i));

                            removal_indices.push_back(value);
                            removal_indices.push_back(i);

                            waiting_coords.erase(location);
                        } else {
                            waiting_coords.insert({ coords, i });
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
                std::cerr << std::flush;
                return;
            }
        }
    }
private:
    // Combine two threads given their indices in m_threads, and return the result of joining them. May operate
    // destructively; the threads are assumed to not be used after this call.
    inline thread<ProgramHolder> join_threads(size_t first_index, size_t second_index) {
        thread<assembly_scanner>& first_thread = m_threads[first_index];
        thread<assembly_scanner>& second_thread = m_threads[second_index];

        std::vector<int24_t> new_stack = join_threads_work(first_thread, second_thread);
        return thread<assembly_scanner>(first_thread, std::move(new_stack));
    }

    inline std::vector<int24_t>
    join_threads_work(thread<ProgramHolder>& first_thread, thread<ProgramHolder>& second_thread) {
        int24_t first_stack_amount = first_thread.m_stack.back();
        first_thread.m_stack.pop_back();

        if (first_thread.m_flags.warnings && first_stack_amount > static_cast<int24_t>(first_thread.m_stack.size()))
            UNLIKELY {
            std::cerr << "Warning: Attempt to move " << first_stack_amount << " values out of thread with stack size "
                      << first_thread.m_stack.size() << '\n';
        }

        int24_t second_stack_amount = second_thread.m_stack.back();
        second_thread.m_stack.pop_back();

        bool whole_second_stack = second_stack_amount < INT24_C(0);

        std::vector<int24_t> new_stack;
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
            std::cerr << "Warning: Attempt to move " << second_stack_amount << " values out of thread with stack size "
                      << second_thread.m_stack.size() << '\n';
        }

        new_stack.insert(
            new_stack.cend(),
            whole_second_stack ? second_thread.m_stack.begin()
                               : (second_thread.m_stack.end() - static_cast<ptrdiff_t>(second_stack_amount)),
            second_thread.m_stack.end()
        );

        return new_stack;
    }

    inline void split_thread(std::vector<thread<ProgramHolder>>& new_threads, const thread<ProgramHolder>& old_thread)
        const {
        thread<ProgramHolder> new_thread_1 = old_thread;
        new_thread_1.m_status = thread<ProgramHolder>::status::active;
        thread<ProgramHolder> new_thread_2 = new_thread_1;

        m_program_holder.advance(new_thread_1.m_ip, []() { return true; });
        m_program_holder.advance(new_thread_2.m_ip, []() { return false; });

        new_threads.push_back(new_thread_1);
        new_threads.push_back(new_thread_2);
    }

    ProgramHolder& m_program_holder;
    std::vector<thread<ProgramHolder>> m_threads;
};

template<>
inline void interpreter<program_walker>::split_thread(
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
inline thread<program_walker> interpreter<program_walker>::join_threads(size_t first_index, size_t second_index) {
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
