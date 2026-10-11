# Setup

## Dependencies

On Windows x86_64, we install MSYS2 as the software distribution and package manager. Then, we can use `pacman` to install MinGW-w64 toolchain, which includes the g++ compiler.
```
# Install MSYS2
winget install --id MSYS2.MSYS2 --exact --accept-package-agreements --accept-source-agreements

$bash = "C:\msys64\usr\bin\bash.exe"

# Update MSYS2 (run twice: the first run may only update the core)
& $bash -lc "pacman -Syu --noconfirm"
& $bash -lc "pacman -Syu --noconfirm"

# Install GCC and the tools into the UCRT64 environment
& $bash -lc "pacman -S --needed --noconfirm mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-gdb mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-ccache mingw-w64-ucrt-x86_64-clang-tools-extra"

# Add the toolchain to your user PATH
$dir = "C:\msys64\ucrt64\bin"
$path = [Environment]::GetEnvironmentVariable("Path", "User")
if (($path -split ";") -notcontains $dir) {
    [Environment]::SetEnvironmentVariable("Path", "$dir;$path", "User")
}
```