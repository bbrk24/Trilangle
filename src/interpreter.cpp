#include "interpreter.hh"

#ifndef __EMSCRIPTEN__
extern "C" void send_thread_count(size_t) {}
#endif
