#pragma once

#include <utility>

#ifdef __has_include
#if __has_include(<version>)
#include <version>
#endif
#endif


#ifdef __cpp_constexpr
#if defined(__cpp_constexpr_dynamic_alloc) && __cpp_constexpr >= 201907
// constexpr if dynamic containers are constexpr
#define CONSTEXPR_ALLOC constexpr
#endif

#ifdef __cpp_constinit
// constinit, constexpr, or neither, depending on where lambdas are allowed
#define CONSTINIT_LAMBDA constinit
#elif __cpp_constexpr >= 201603
// constinit, constexpr, or neither, depending on where lambdas are allowed
#define CONSTINIT_LAMBDA constexpr
#endif
#endif

#ifndef CONSTEXPR_ALLOC
// constexpr if dynamic containers are constexpr
#define CONSTEXPR_ALLOC inline
#endif

#ifndef CONSTINIT_LAMBDA
// constinit, constexpr, or neither, depending on where lambdas are allowed
#define CONSTINIT_LAMBDA
#endif


#ifdef __has_cpp_attribute
#if __has_cpp_attribute(maybe_unused)
// explicitly throw out the result of a [[nodiscard]] function
#define DISCARD [[maybe_unused]] auto _ =
#endif

#if __has_cpp_attribute(unlikely)
// mark an execution branch as being very unlikely, and that it's acceptable for it to be slow
#define UNLIKELY [[unlikely]]
#endif

#if __has_cpp_attribute(assume)
#define __HAS_ASSUME
#endif

#if __has_cpp_attribute(fallthrough)
// mark an intentional lack of a break statement in a switch block
#define FALLTHROUGH [[fallthrough]];
#endif
#endif

#ifndef DISCARD
// explicitly throw out the result of a [[nodiscard]] function
#define DISCARD (void)
#endif

#ifndef UNLIKELY
// mark an execution branch as being very unlikely, and that it's acceptable for it to be slow
#define UNLIKELY
#endif

#ifndef FALLTHROUGH
// mark an intentional lack of a break statement in a switch block
#define FALLTHROUGH
#endif


#ifdef __cpp_noexcept_function_type
// noexcept if it's part of the type
#define NOEXCEPT_T noexcept
// noexcept if it's part of the type
#else
#define NOEXCEPT_T
#endif


#ifdef __cpp_size_t_suffix
// mark a literal as a size_t
#define SIZE_C(x) x ## UZ
#else
// mark a literal as a size_t
#define SIZE_C(x) x ## U
#endif

#ifdef __cpp_lib_unreachable
using std::unreachable;
#elif defined(__HAS_ASSUME)
#define unreachable() [[assume(false)]]
#else
[[noreturn]] static inline void unreachable() {
    // Polyfill derived from https://en.cppreference.com/w/cpp/utility/unreachable
#ifdef __GNUC__
    __builtin_unreachable();
#elif defined(_MSC_VER)
    __assume(0);
#endif
}
#endif

#undef __HAS_ASSUME
