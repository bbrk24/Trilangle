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
#define EX_USAGE 64
#endif

#ifndef EX_NOINPUT
#define EX_NOINPUT 66
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
#define SIZE_C(x) x##UZ
#else
// mark a literal as a size_t
#define SIZE_C(x) x##U
#endif


#if !defined(__GNUC__) && defined(_MSC_VER)
#define __builtin_unreachable() __assume(0)
#endif

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
