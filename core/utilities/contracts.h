#ifndef CONTRACTS_H
#define CONTRACTS_H

/// Contract hardening level (0: none, 1: all, default: debug-only)
#ifndef XSTD_HARDENING_MODE_NONE
#define XSTD_HARDENING_MODE_NONE 0
#define XSTD_HARDENING_MODE_ALL 1
#endif 

#ifndef XSTD_HARDENING_MODE
#define XSTD_HARDENING_MODE XSTD_HARDENING_MODE_NONE
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

#endif