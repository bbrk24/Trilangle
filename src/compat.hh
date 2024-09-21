#pragma once

#include <cassert>
#include <utility>

#ifdef __has_include
#if __has_include(<sal.h>) && !defined(__clang__)
#include <sal.h>
#define _INCLUDED_SAL
#endif

#if __has_include(<sysexits.h>)
#include <sysexits.h>
#endif

#if __has_include(<stdfloat>)
#include <stdfloat>
#endif

#if __has_include(<version>)
#include <version>
#define _INCLUDED_VERSION
#endif
#endif

#ifndef _INCLUDED_VERSION
#include <algorithm>
#include <string>
#include <vector>
#endif

#undef _INCLUDED_VERSION

#ifndef EX_USAGE
constexpr int EX_USAGE = 64;
#endif

#ifndef EX_DATAERR
constexpr int EX_DATAERR = 65;
#endif

#ifndef EX_NOINPUT
constexpr int EX_NOINPUT = 66;
#endif


#ifdef __STDCPP_FLOAT16_T__
// The smallest supported float type.
typedef std::float16_t small_float;
#elif defined(__STDCPP_BFLOAT16_T__)
// The smallest supported float type.
typedef std::bfloat16_t small_float;
#else
// The smallest supported float type.
typedef float small_float;
#endif


#if !defined(__GNUC__) && defined(_MSC_VER)
// 1 if the compiler is really MSVC, and not clang pretending to be MSVC. 0 for clang and GCC.
#define REALLY_MSVC 1
#else
// 1 if the compiler is really MSVC, and not clang pretending to be MSVC. 0 for clang and GCC.
#define REALLY_MSVC 0
#endif


#ifdef __cpp_constinit
// constinit, constexpr, or just const, depending on where lambdas are allowed
#define CONSTINIT_LAMBDA constinit const
#elif defined(__cpp_constexpr)
#if __cpp_constexpr >= 201603
// constinit, constexpr, or just const, depending on where lambdas are allowed
#define CONSTINIT_LAMBDA constexpr
#endif
#endif

#ifndef CONSTINIT_LAMBDA
// constinit, constexpr, or just const, depending on where lambdas are allowed
#define CONSTINIT_LAMBDA const
#endif


#ifdef __cpp_lib_constexpr_vector
// constexpr if std::vector is constexpr
#define CONSTEXPR_VECTOR constexpr

#ifdef __cpp_lib_constexpr_string
// constexpr if std::vector and std::string are constexpr
#define CONSTEXPR_ALLOC constexpr
#ifdef __cpp_lib_constexpr_algorithms
// constexpr if functions in <algorithm> are constexpr
#define CONSTEXPR_ALG constexpr
#endif
#endif
#else
// constexpr if std::vector is constexpr
#define CONSTEXPR_VECTOR inline
#endif

#ifndef CONSTEXPR_ALLOC
// constexpr if std::vector and std::string are constexpr
#define CONSTEXPR_ALLOC inline
#endif

#ifndef CONSTEXPR_ALG
// constexpr if functions in <algorithm> are constexpr
#define CONSTEXPR_ALG inline
#endif


#ifdef __cpp_constexpr
#if __cpp_constexpr >= 202002L
// constexpr if unions can be used constexpr
#define CONSTEXPR_UNION constexpr
#endif
#endif

#ifndef CONSTEXPR_UNION
// constexpr if unions can be used constexpr
#define CONSTEXPR_UNION inline
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

#if REALLY_MSVC
#if _MSC_VER >= 1929
// mark that the padding of this member can be used for other members
#define NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#endif
#elif __has_cpp_attribute(no_unique_address)
// mark that the padding of this member can be used for other members
#define NO_UNIQUE_ADDRESS [[no_unique_address]]
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

#ifndef NO_UNIQUE_ADDRESS
// mark that the padding of this member can be used for other members
#define NO_UNIQUE_ADDRESS
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
#define SIZE_C(x) x##UZ
#else
// mark a literal as a size_t
#define SIZE_C(x) x##U
#endif


#if REALLY_MSVC
#define __builtin_unreachable() __assume(0)
#endif

// Indicate that a code path should not be reachable. In debug builds, this causes an assertion failure, including the
// given reason in the error message. In optimized builds, this expands to a single __builtin_unreachable call.
#define unreachable(reason) \
    assert("Assumption violated: " reason && false); \
    __builtin_unreachable()


#ifdef __clang__
#pragma clang diagnostic ignored "-Wnullability-completeness"
// A non-nullable pointer to the specified type.
#define NONNULL_PTR(...) __VA_ARGS__* _Nonnull
// A null-terminated, read-only array of characters. Can only be used for parameters, use `NONNULL_PTR(const char)`
// elsewhere.
#define CONST_C_STR const char* _Nonnull
#elif defined(_INCLUDED_SAL)
// A non-nullable pointer to the specified type.
#define NONNULL_PTR(...) _Notnull_ __VA_ARGS__*
// A null-terminated, read-only array of characters. Can only be used for parameters, use `NONNULL_PTR(const char)`
// elsewhere.
#define CONST_C_STR _In_z_ const char*
#else
// A non-nullable pointer to the specified type.
#define NONNULL_PTR(...) __VA_ARGS__*
// A null-terminated, read-only array of characters. Can only be used for parameters, use `NONNULL_PTR(const char)`
// elsewhere.
#define CONST_C_STR const char*
// gcc's __attribute__((nonnull)) works differently, so I can't wrap it like this.
#endif

#ifndef _INCLUDED_SAL
#define _In_reads_z_(s)
#endif

#undef _INCLUDED_SAL
