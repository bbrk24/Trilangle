#include "thread.hh"

using std::cout;

unsigned long thread_count;

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

    DISCARD getchar();
}
#endif
