#pragma once

constexpr const char* header =
#include "lightweight_stack.h"
    R"#(
#include <stdio.h>
#include <inttypes.h>
#include <wchar.h>

int main(int argc, const char** argv) {
    struct lightweight_stack_s stack_storage;
    lws stack = &stack_storage;
    lws_init(stack);
)#";

constexpr const char* footer = "}\n";
