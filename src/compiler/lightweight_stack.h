// This entire file is a string. Comment out the opening quote marker to get syntax highlighting while editing.
R"(
#include <stdlib.h>
#include <stdio.h>
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

#define INVALID_CHAR 0xfffd
// Basically copied from parse_unichar in string_processing.hh
static inline int32_t get_unichar() {
    unsigned char buf[4];
    size_t buf_max = 4;
    for (size_t i = 0; i < buf_max; ++i) {
        int c = getchar();
        if (c == EOF) {
            if (i == 0) {
                return -1;
            } else {
                return INVALID_CHAR;
            }
        }
        buf[i] = static_cast<unsigned char>(c);
        if (i != 0 && (buf[i] & 0xc0) != 0x80) {
            return INVALID_CHAR;
        }
        if (i == 0) {
            if (buf[0] < 0x80) {
                return buf[0];
            } else if ((buf[0] & 0xe0) == 0xc0) {
                buf_max = 2;
            } else if ((buf[0] & 0xf0) == 0xe0) {
                buf_max = 3;
            } else if ((buf[0] & 0xf8) != 0xf0) {
                return INVALID_CHAR;
            }
        }
    }
    switch (buf_max) {
        case 2:
            return ((buf[0] & 0x1f) << 6) | (buf[1] & 0x3f);
        case 3:
            return ((buf[0] & 0x0f) << 12) | ((buf[1] & 0x3f) << 6) | (buf[2] & 0x3f);
        case 4:
            return ((buf[0] & 0x07) << 18) | ((buf[1] & 0x3f) << 12) | ((buf[2] & 0x3f) << 6) | (buf[3] & 0x3f);
    }
}

static inline void print_unichar(int32_t c) {
    if (c <= 0x7f) {
        putchar(c);
    } else if (c <= 0x07ff) {
        char buffer[] = { 0xc0 | (c >> 6), 0x80 | (c & 0x3f), 0 };
        printf(buffer);
    } else if (c <= 0xffff) {
        char buffer[] = { 0xe0 | (c >> 12), 0x80 | ((c >> 6) & 0x3f), 0x80 | (c & 0x3f), 0 };
        printf(buffer);
    } else {
        char buffer[] = { 0xf0 | (c >> 18), 0x80 | ((c >> 12) & 0x3f), 0x80 | ((c >> 6) & 0x3f), 0x80 | (c & 0x3f), 0 };
        printf(buffer);
    }
}
//)"
