// This entire file is a string. Comment out the opening quote marker to get syntax highlighting while editing.
R"(
#include <stdlib.h>
#include <stdint.h>

#ifndef __GNUC__
#define __builtin_expect(exp, c) exp
#endif

struct lightweight_stack_s {
    int32_t* base;
    size_t length;
    size_t capacity;
};

typedef struct lightweight_stack_s* lws;

static inline int32_t lws_pop(lws stack) {
    return stack->base[--stack->length];
}

static inline int lws_push(lws stack, int32_t value) {
    if (__builtin_expect(stack->length == stack->capacity, 0L)) {
        size_t new_cap = stack->capacity * 7 / 4;
        int32_t* new_ptr = (int32_t*)realloc(stack->base, new_cap * sizeof(int32_t));
        if (new_ptr == NULL) {
            return 0;
        }
        stack->base = new_ptr;
        stack->capacity = new_cap;
    }
    stack->base[stack->length++] = value;
    return 1;
}

static inline int32_t lws_index(const lws stack, int32_t index) {
    return stack->base[stack->length - index - 1];
}

static inline int32_t lws_top(const lws stack) {
    return lws_index(stack, 0);
}

static inline void lws_init(lws stack) {
    stack->length = 0;
    stack->base = (int32_t*)calloc(4, sizeof(int32_t));
    // Note: actually handling errors prevents the compiler from doing optimizations we want
    stack->capacity = 4;
}

static inline void lws_deinit(const lws stack) {
    free(stack->base);
}
//)"
