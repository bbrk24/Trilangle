#define _CRT_SECURE_NO_WARNINGS 1

#include "thread.hh"

using std::cout;

unsigned long thread_count;

template<class ProgramHolder>
int32_t thread<ProgramHolder>::read_int() noexcept {
    int32_t i = -1;

    while (!(feof(stdin) || scanf("%" SCNi32, &i))) {
        [[maybe_unused]] int _ = getchar();
    }

    return i;
}

#ifndef __EMSCRIPTEN__
extern "C" void send_debug_info(
    unsigned long thread_number,
    const int24_t* stack,
    size_t stack_depth,
    size_t y,
    size_t x,
    CONST_C_STR instruction
) {
    cout << "Thread " << thread_number << '\n';
    if (stack != nullptr) {
        cout << "Stack: [";

        for (size_t i = 0; i < stack_depth; ++i) {
            if (i != 0) {
                cout << ", ";
            }
            cout << stack[i];
        }

        cout << "]\n";
    }

    cout << "Coords: (" << x << ", " << y << ")\nInstruction: " << instruction << std::endl;

    [[maybe_unused]] int _ = getchar();
}
#endif
