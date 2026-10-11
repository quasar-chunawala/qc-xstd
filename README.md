# `qc::xstd`
A collection of standard containers, RAII wrappers for modern C++ and used in quantitative trading and finance.

## Compatibility

- **Language Standard**: C++26
- **Compilers Tested**: GCC 16.2

## Integration

This library is header-only and can be copied directly into your project or fetched via CMake.

### CMake FetchContent

```
include(FetchContent)

FetchContent_Declare(
  qc_xstd
  GIT_REPOSITORY https://github.com/quasar-chunawala/qc-xstd.git
  GIT_TAG        v1.0.0
)
FetchContent_MakeAvailable(xstd)

target_link_libraries(your_target PRIVATE xstd)
```

## Quick Start


# Setup

- For Linux, follow the instructions in [SetupLinux.md](./SetupLinux.md).

- For Win64, refer [SetupWin64.md](./SetupWin64.md).

- For MacOS, refer [SetupMacOS.md](./SetupMacOS.md)