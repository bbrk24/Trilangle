#pragma once

#include <utility>

#ifdef __has_include
#if __has_include(<version>)
#include <version>
#define __INCLUDED_VERSION
#endif
#endif

#ifndef __INCLUDED_VERSION
#include <vector>
#include <string>
#endif

#undef __INCLUDED_VERSION


#ifdef __cpp_constinit
// constinit, constexpr, or neither, depending on where lambdas are allowed
#define CONSTINIT_LAMBDA constinit
#elif defined(__cpp_constexpr)
#if __cpp_constexpr >= 201603
// constinit, constexpr, or neither, depending on where lambdas are allowed
#define CONSTINIT_LAMBDA constexpr
#endif
#endif

#ifndef CONSTINIT_LAMBDA
// constinit, constexpr, or neither, depending on where lambdas are allowed
#define CONSTINIT_LAMBDA
#endif

#ifdef __cpp_constexpr_dynamic_alloc
// constexpr if destructors are allowed to be
#define CONSTEXPR_DESTRUCT constexpr
#else
// constexpr if destructors are allowed to be
#define CONSTEXPR_DESTRUCT inline
#endif


#if defined(__cpp_lib_constexpr_vector) && defined(__cpp_lib_constexpr_string)
// constexpr if dynamic containers are constexpr
#define CONSTEXPR_ALLOC constexpr

#ifdef __cpp_lib_constexpr_algorithms
// constexpr if functions in <algorithm> are constexpr
#define CONSTEXPR_ALG constexpr
#endif

#else
// constexpr if dynamic containers are constexpr
#define CONSTEXPR_ALLOC inline
#endif

#ifndef CONSTEXPR_ALG
// constexpr if functions in <algorithm> are constexpr
#define CONSTEXPR_ALG inline
#endif


#ifdef __has_cpp_attribute
#if __has_cpp_attribute(maybe_unused)
// mark that it's okay if a function is never called
#define MAYBE_UNUSED [[maybe_unused]]

// explicitly throw out the result of a [[nodiscard]] function
#define DISCARD [[maybe_unused]] auto _ =
#endif

#if __has_cpp_attribute(unlikely)
// mark an execution branch as being very unlikely, and that it's acceptable for it to be slow
#define UNLIKELY [[unlikely]]
#endif

#if __has_cpp_attribute(fallthrough)
// mark an intentional lack of a break statement in a switch block
#define FALLTHROUGH [[fallthrough]];
#endif
#endif

#ifndef MAYBE_UNUSED
// mark that it's okay if a function is never called
#define MAYBE_UNUSED
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
#else
// noexcept if it's part of the type
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

#ifdef __has_attribute
#if __has_attribute(packed)
#define PACK(...) __VA_ARGS__ __attribute__((packed))
#endif
#endif

#ifndef PACK
#ifdef _MSC_VER
#define PACK(...) _Pragma("pack(push, 1)") __VA_ARGS__ _Pragma("pack(pop)")
#else
#define PACK(...) __VA_ARGS__
#endif
#endif
