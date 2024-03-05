// This entire file is a string. Comment out the opening quote marker to get syntax highlighting while editing.
R"(
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

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

static const int32_t INVALID_CHAR = 0xfffd;

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
        buf[i] = c;
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
    // Should be unreachable but the compiler doesn't know better
    return INVALID_CHAR;
}

static inline void print_unichar(int32_t c) {
    if (c <= 0x7f) {
        putchar(c);
    } else if (c <= 0x07ff) {
        char buffer[] = { 0xc0 | (c >> 6), 0x80 | (c & 0x3f), 0 };
        printf("%s", buffer);
    } else if (c <= 0xffff) {
        char buffer[] = { 0xe0 | (c >> 12), 0x80 | ((c >> 6) & 0x3f), 0x80 | (c & 0x3f), 0 };
        printf("%s", buffer);
    } else {
        char buffer[] = { 0xf0 | (c >> 18), 0x80 | ((c >> 12) & 0x3f), 0x80 | ((c >> 6) & 0x3f), 0x80 | (c & 0x3f), 0 };
        printf("%s", buffer);
    }
}

static const time_t SECS_PER_DAY = 86400;

static inline int32_t get_time() {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    const double UNITS_PER_NSEC = 9.709037037036571e-8;
    const double UNITS_PER_SEC = 97.0903703703657;
    double value = UNITS_PER_SEC * (double)(ts.tv_sec % SECS_PER_DAY) + UNITS_PER_NSEC * (double)ts.tv_nsec;
    return (int32_t)value;
}

static inline int32_t get_date() {
    time_t t = time(NULL);
    return (int32_t)(t / SECS_PER_DAY);
}

typedef struct {
    uint32_t x[5];
    uint32_t counter;
} xorwow_state;

static inline int32_t rand24() {
#if RAND_MAX <= 0xffff
#define RAND32() (rand() << 16) | rand()
#else
#define RAND32() rand()
#endif
    static xorwow_state state = {
        { RAND32(), RAND32(), RAND32() | 0x00008000, RAND32(), RAND32() },
        0
    };
#undef RAND32

    /* Algorithm "xorwow" from p. 5 of Marsaglia, "Xorshift RNGs" */
    uint32_t t  = state.x[4];

    uint32_t s  = state.x[0];  /* Perform a contrived 32-bit shift. */
    state.x[4] = state.x[3];
    state.x[3] = state.x[2];
    state.x[2] = state.x[1];
    state.x[1] = s;

    t ^= t >> 2;
    t ^= t << 1;
    t ^= s ^ (s << 4);
    state.x[0] = t;
    state.counter += 362437;
    // Deviation from actual xorwow algorithm: shift out the low byte to get only 24 bits
    return (int32_t)(t + state.counter) >> 8;
}
//)"
