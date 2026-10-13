# `qc::xstd`
A collection of standard containers, RAII wrappers for modern C++ and used in quantitative trading and finance.

## Configure and Build

To list all presets, execute:

```bash
cmake --list-presets
```

To configure and build the code, run:

```
cmake --preset <preset-name>
cmake --build --preset <preset-name>
```

- To install dependencies for Linux, follow the instructions in [SetupLinux.md](./SetupLinux.md).

- To install dependencies for Win64, refer [SetupWin64.md](./SetupWin64.md).

- To install dependencies for MacOS, refer [SetupMacOS.md](./SetupMacOS.md)

## Compatibility

- **Language Standard**: C++23
  - *Note*: `std::meta` requires C++26.
- **Compilers Tested**: 
  - Clang 22
  - GCC 15, 16.2
  - MSVC 17(VS 2022), 18(2026)
  

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

## Configuration Macros

### Hardening and Contract Checks

Runtime precondition checks (null pointersm out-of-bounds indices, invalid iterator ranges) can be configured globally:

| Macro | Options / Default | Description |
|---|---|---|
| `QX_HARDENING_MODE` | `QX_HARDENING_MODE_OFF` (Default)<br>`QX_HARDENING_MODE_ALL` | Enables precondition checks across all API boundaries. |
| `QX_ASSERT_MODE` <br> `QX_ASSERT_MODE_LOG_TRAP` (Default)<br>`QX_ASSERT_MODE_NONE`<br>`QX_ASSERT_MODE_TRAP`<br>`QX_ASSERT_MODE_ABORT`<br>`QX_ASSERT_MODE_LOG_ABORT` | Defines failure action when a hardening assertion triggers. |