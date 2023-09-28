#pragma once

constexpr const char* header =
    "#include <inttypes.h>"
#include "lightweight_stack.h"
    R"#(
int main(int argc, const char** argv) {
	struct lightweight_stack_s stack_storage;
	lws stack = &stack_storage;
	lws_init(stack);)#";

constexpr const char* footer = "}\n";
