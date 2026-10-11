# Setup

The VSCode extensions C/C++ extension pack, clangd, CMake Tools, CMake Language Support, CodeLLDB, CS 128 Clang-Tidy, Git Blame, Git Graph, Git History, VSCode Remote extension pack,
Clang-Format must be installed.

## Dependencies

In a new powershell window, install `git`.

```powershell
winget install --id Git.Git -e --source winget
```

On Windows x86_64, we will install MSYS2 as the software distribution and package manager. Then, we use `pacman` to install MinGW-w64 toolchain, which includes the g++ compiler. Save the below script at `C:\dev\setup.ps1`.

```powershell
# c:/dev/setup.ps1

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

Again, in a new powershell window, execute the following commands:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
C:\dev\setup.ps1
```

## Recommended VS Code Setup

I recommend creating `.vscode/extensions.json` file under the worktree. The below VS Code extensions are really useful:

### `.vscode/extensions.json`
```json
{
    "recommendations": [
        "llvm-vs-code-extensions.vscode-clangd",
        "ms-vscode.cpptools-extension-pack",
        "ms-vscode-remote.vscode-remote-extensionpack",
        "cs128.cs128-clang-tidy",
        "donjayamanne.githistory",
        "josetr.cmake-language-support-vscode",
        "mhutchie.git-graph",
        "ms-vscode.cmake-tools",
        "vadimcn.vscode-lldb",
        "waderyan.gitblame",
        "xaver.clang-format"
    ]
}
```

### `.vscode/settings.json`

```json
{
    "editor.formatOnSave": true,
    "[cmake]": {
        "editor.quickSuggestions": {
            "strings": true
        },
        "editor.defaultFormatter": "llvm-vs-code-extensions.vscode-clangd"
    },
    "[cpp]": {
        "editor.wordBasedSuggestions": "off",
        "editor.semanticHighlighting.enabled": true,
        "editor.stickyScroll.defaultModel": "foldingProviderModel",
        "editor.suggest.insertMode": "replace",
        "editor.defaultFormatter": "llvm-vs-code-extensions.vscode-clangd"
    },
    "cmake.useCMakePresets": "always",
    "cmake.ctest.testExplorerIntegrationEnabled": true,
    "cmake.copyCompileCommands": "${workspaceFolder}/compile_commands.json",
    "clangd.arguments": [
        "--compile-commands-dir=${workspaceFolder}"
    ],
    "cmake.options.statusBarVisibility": "visible"
}
```