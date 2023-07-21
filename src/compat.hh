#pragma once

#include <cassert>
#include <utility>

#ifdef __has_include
#if __has_include(<sal.h>)
#include <sal.h>
#endif

#if __has_include(<sysexits.h>)
#include <sysexits.h>
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

#ifndef EX_NOINPUT
constexpr int EX_NOINPUT = 66;
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


// There's no macro explicitly for checking if constexpr destructors are allowed, but constexpr allocation requires it
#ifdef __cpp_constexpr_dynamic_alloc
// constexpr if destructors are allowed to be
#define CONSTEXPR_DESTRUCT constexpr
#else
// constexpr if destructors are allowed to be
#define CONSTEXPR_DESTRUCT inline
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

#if !defined(__GNUC__) && defined(_MSC_VER)
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


#if !defined(__GNUC__) && defined(_MSC_VER)
#define __builtin_unreachable() __assume(0)
#endif

// Indicate that a code path should not be reachable. In debug builds, this causes an assertion failure, including the
// given reason in the error message. In optimized builds, this expands to a single __builtin_unreachable call.
#define unreachable(reason) \
    assert(("Assumption violated: " reason, false)); \
    __builtin_unreachable()


#ifdef __clang__
// A non-nullable pointer to the specified type.
#define NONNULL_PTR(...) __VA_ARGS__* _Nonnull
#elif defined(_Notnull_)
// A non-nullable pointer to the specified type.
#define NONNULL_PTR(...) _Notnull_ __VA_ARGS__*
#else
// A non-nullable pointer to the specified type.
#define NONNULL_PTR(...) __VA_ARGS__*
// gcc's __attribute__((nonnull)) works differently, so I can't wrap it like this.
#endif


#if (defined(__x86_64__) || defined(_M_X64)) && (defined(__GNUC__) || defined(__clang__))
// 1 on gcc or clang for x86-64, where asm blocks are used. 0 otherwise.
#define ASM_ALLOWED 1
#else
// 1 on gcc or clang for x86-64, where asm blocks are used. 0 otherwise.
#define ASM_ALLOWED 0
#endif
