#include <core/configuration/hardening.h>

#ifndef CONTRACTS_H
#define CONTRACTS_H

// A contract is a formal interface specification for a software
// component such as a class or member-function. It is a set of
// conditions that express expectations on how the component
// interoperates with other components of the program.
//
// A contract violation occurs when a condition that is a part of
// the contract does not hold.
//
// A pre-condition is part of the function contract where the
// responsibility of satisfying it rests with the caller. Generally,
// these requirements are placed on the arguments passed to the function
// upon entry into the function.
//
// A postcondition is a part of the function contract where the responsibiluty
// of satisfying the condition rests with the callee; the implementer of
// the function itself. These are generally conditions that will hold
// true regarding the return value of the function when it completes
// execution normally.
//
// An invariant is a condition on the state ofan object, or a set of
// obsets, that is maintained over a certain amount of time.
// A class invariant is a condition that a class type maintains throughout
// the lifetime of an object. There are other kinds of invariants such as
// loop invariants. Often these are expected to hold on the entry
// or exit of functions

// qc::xstd provides several hardening modes, where each mode enables a
// set of assertions that prevent UB caused by contract violations.
// of the library. Different hardening modes make different tradeoff between
// checking and runtime performance.

#if XSTD_HARDENING_MODE != XSTD_HARDENING_MODE_NONE &&                         \
    XSTD_HARDENING_MODE != XSTD_HARDENING_MODE_FAST &&                         \
    XSTD_HARDENING_MODE != XSTD_HARDENING_MODE_EXTENSIVE &&                    \
    XSTD_HARDENING_MODE != XSTD_HARDENING_MODE_DEBUG
#error _LIBCPP_HARDENING_MODE must be set to one of the following values: \
XSTD_HARDENING_MODE_NONE, \
XSTD_HARDENING_MODE_FAST, \
XSTD_HARDENING_MODE_EXTENSIVE, \
XSTD_HARDENING_MODE_DEBUG
#endif

// contract assert behavior (default: IO-logging and trap)
#ifndef XSTD_ASSERT_MODE_NONE
#define XSTD_ASSERT_MODE_NONE 0
#define XSTD_ASSERT_MODE_TRAP 1
#define XSTD_ASSERT_MODE_LOG_TRAP 2
#define XSTD_ASSERT_MODE_ABORT 3
#define XSTD_ASSERT_MODE_LOG_ABORT 4
#endif

#ifndef XSTD_ASSERT_MODE
#define XSTD_ASSERT_MODE XSTD_ASSERT_MODE_LOG_TRAP // default assertion mode
#else
#if (XSTD_ASSERT_MODE > XSTD_ASSERT_MODE_LOG_ABORT)
#undef XSTD_ASSERT_MODE
#define XSTD_ASSERT_MODE XSTD_ASSERT_MODE_NONE
#endif
#endif

// Definition of XSTD_TRAP macro
// __builtin_trap is a GCC + clang builtin
#ifndef XTSD_TRAP
#if __has_builtin(__builtin_trap) || defined(__GNUC__)
#define XSTD_TRAP __builtin_trap()
#elif defined(_MSC_VER)
// __debugbreak() alone doesn't mark the site as noreturn to the optimizer;
// __assume(false) tells MSVC that the control never returns
#define XSTD_TRAP (__debugbreak(), __assume(false))
#else
#define XSTD_TRAP std::abort()
#endif
#endif

// Definition of XSTD_TRAP_MSG macro

#endif