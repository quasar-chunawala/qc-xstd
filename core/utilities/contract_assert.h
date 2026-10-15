#ifndef CONTRACTS_H
#define CONTRACTS_H

#include <core/configuration/hardening.h>
#include <cstdio>
#include <cstdlib>

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
// these requirements are placed on the arguments passed to the
// function upon entry into the function.
//
// A postcondition is a part of the function contract where the
// responsibiluty of satisfying the condition rests with the callee;
// the implementer of the function itself. These are generally
// conditions that will hold true regarding the return value of the
// function when it completes execution normally.
//
// An invariant is a condition on the state ofan object, or a set of
// obsets, that is maintained over a certain amount of time.
// A class invariant is a condition that a class type maintains
// throughout the lifetime of an object. There are other kinds of
// invariants such as loop invariants. Often these are expected to hold
// on the entry or exit of functions

// qc::xstd provides several hardening modes, where each mode enables a
// set of assertions that prevent UB caused by contract violations.
// of the library. Different hardening modes make different tradeoffs
// between checking and runtime performance.

// If the user does not configure the hardening level, default to
// XSTD_HARDENING_MODE_NONE
#ifndef XSTD_HARDENING_MODE
	#define XSTD_HARDENING_MODE XSTD_HARDENING_MODE_NONE
#endif

// If the hardening level chosen by the user is invalid, error out
#if XSTD_HARDENING_MODE != XSTD_HARDENING_MODE_NONE &&                    \
    XSTD_HARDENING_MODE != XSTD_HARDENING_MODE_FAST &&                    \
    XSTD_HARDENING_MODE != XSTD_HARDENING_MODE_EXTENSIVE &&               \
    XSTD_HARDENING_MODE != XSTD_HARDENING_MODE_DEBUG
	#error XSTD_HARDENING_MODE must be set to one of the following values: \
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
	#define XSTD_ASSERT_MODE                                              \
		XSTD_ASSERT_MODE_LOG_TRAP // default assertion mode
#else
	#if (XSTD_ASSERT_MODE > XSTD_ASSERT_MODE_LOG_ABORT)
		#undef XSTD_ASSERT_MODE
		#define XSTD_ASSERT_MODE XSTD_ASSERT_MODE_NONE
	#endif
#endif

// Definition of XSTD_TRAP macro
// __builtin_trap is a GCC + clang builtin
#ifndef XSTD_TRAP
	#if __has_builtin(__builtin_trap) || defined(__GNUC__)
		#define XSTD_TRAP() __builtin_trap()
	#elif defined(_MSC_VER)
		// __debugbreak() alone doesn't mark the site as noreturn to
		// the optimizer;
		// __assume(false) tells MSVC that the control never returns
		#define XSTD_TRAP() (__debugbreak(), __assume(false))
	#else
		#define XSTD_TRAP() std::abort()
	#endif
#endif

namespace xstd {

	struct AssertViolation {
		const char *expression_; // stringified condition
		const char *comment_;    // user message
		const char *file_name_;
		int line_number_;
		const char *category_;

		[[nodiscard]] const char *expression() const {
			return expression_;
		}
		[[nodiscard]] const char *file_name() const { return file_name_; }

		[[nodiscard]] const char *comment() const { return comment_; }

		[[nodiscard]] int line_number() const { return line_number_; }

		[[nodiscard]] const char *category() const { return category_; }
	};

	using ViolationHandler = void (*)(const AssertViolation &);

} // namespace xstd

// Definition of XSTD_TRAP_WITH_MSG macro
// __builtin_verbose_trap is clang only
#ifndef XSTD_TRAP_WITH_MSG
	#if defined(__clang__) && defined(__has_builtin)
		#if __has_builtin(__builtin_verbose_trap)
			#if defined(__apple_build_version__) &&                       \
			    __apple_build_version__ < 17000000
				#define XSTD_TRAP_WITH_MSG(tag, msg)                      \
					__builtin_verbose_trap(tag ", " msg)
			#else
				#define XSTD_TRAP_WITH_MSG(tag, msg)                      \
					__builtin_verbose_trap(tag, msg)
			#endif
		#endif
	#else
		#define XSTD_TRAP_WITH_MSG(tag, msg) XSTD_TRAP()
	#endif
#endif

#ifndef XSTD_STRINGIFY
	#define XSTD_STRINGIFY_IMPL(x) #x
	#define XSTD_STRINGIFY(x) XSTD_STRINGIFY_IMPL(x)
#endif

#define XSTD_VIOLATION(cond, category, msg)                               \
	::xstd::AssertViolation(#cond, msg, __FILE__, __LINE__, category)

namespace xstd {
	struct Assert {
		static void contract_fail_handler_default(
		    const xstd::AssertViolation &violation) {
			std::fprintf(stderr,
			             "%s:%d: contract violation: %s\n",
			             violation.file_name(),
			             violation.line_number(),
			             violation.comment());
			std::fflush(stderr);
		}

		static void set_contract_fail_handler(ViolationHandler handler) {
			violationHandler = handler;
		}

		static void
		contract_fail_handler(const AssertViolation &violation) {
			(*violationHandler)(violation);
		}

	private:
		static inline ViolationHandler violationHandler{
		    &contract_fail_handler_default};
	};
} // namespace xstd

#if XSTD_ASSERT_MODE == XSTD_ASSERT_MODE_NONE
	#define XSTD_CONTRACT_ASSERT(cond, category, msg) ((void)0)
#elif XSTD_ASSERT_MODE == XSTD_ASSERT_MODE_TRAP
	#define XSTD_ON_FAILURE(cond, category, msg) XSTD_TRAP()
#elif XSTD_ASSERT_MODE == XSTD_ASSERT_MODE_LOG_TRAP
// TODO : Implement LOG_TRAP case
//	#define XSTD_ON_FAILURE(cond, category, msg)                          \
//(::xstd::Assert::contract_fail_handler()XSTD_VIOLATION(cond, category,
// msg),
// XSTD_TRAP_WITH_MSG(#cond, __FILE__
//                   ":" XSTD_STRINGIFY(__LINE__) ": " msg category))
#elif XSTD_ASSERT_MODE == XSTD_ASSERT_MODE_ABORT
	#define XSTD_ON_FAILURE(cond, category, msg) std::abort()
#elif XSTD_ASSERT_MODE == XSTD_ASSER_MODE_LOG_ABORT
	#define XSTD_ON_FAILURE(cond, category, msg) std::abort()
#endif

#if XSTD_ASSERT_MODE == XSTD_ASSERT_MODE_NONE
	#define XSTD_CONTRACT_ASSERT(cond, category, msg) ((void)0)
#else
	#define XSTD_CONTRACT_ASSERT(cond, category, msg)                     \
		((cond) ? (void)0 : XSTD_ON_FAILURE(cond, category, msg))
#endif

#endif