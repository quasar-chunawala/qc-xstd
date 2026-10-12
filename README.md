# `qc::xstd`
A collection of standard containers, RAII wrappers for modern C++ and used in quantitative trading and finance.

## Configure and Build

### Linux

```bash
cmake --preset linux-gcc16-x64-debug            # Configure step
cmake --build --preset linux-gcc16-x64-debug    # Build step
```

To install dependencies for Linux, follow the instructions in [SetupLinux.md](./BuildLinux.md).

### Win64 MSVC(Microsoft Visual C++) toolchain

#### Visual Studio 2022

```bash
cmake --preset windows-msvc2022-x64-debug           # Configure step
cmake --build --preset windows-msvc2022-x64-debug    # Build step
```

#### Visual Studio 2026

```bash
cmake --preset windows-msvc2026-x64-debug           # Configure step
cmake --build --preset windows-msvc2026-x64-debug    # Build step
```

###  Win64 GNU toolchain

```bash
cmake --preset win64-gcc16-x64-debug            # Configure step
cmake --build --preset win64-gcc16-x64-debug    # Build step
```

To install dependencies for Win64, refer [SetupWin64.md](./SetupWin64.md).

### MacOS

```bash
cmake --preset macos-gcc16-debug            # Configure step
cmake --build --preset macos-gcc16-debug    # Build step
```

To install dependencies for MacOS, refer [SetupMacOS.md](./SetupMacOS.md)

## Compatibility

- **Language Standard**: C++26
- **Compilers Tested**: GCC 16.2

## Integration

This library is header-only and can be copied directly into your project or fetched via CMake.

### CMake FetchContent

```cmake
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
