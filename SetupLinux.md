# Setup

## Dependencies

On ArchLinux:

```bash
LLVM_VERSION=22
GCC_VERSION=16
sudo pacman -Syu base-devel cmake ninja clang${LLVM_VERSION} llvm${LLVM_VERSION} gdb lldb ccache valgrind git code
```

On Ubuntu:



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

## Configure and Build

There are various configuration presets defined in `/CMakePresets.json`. To configure:

```
cmake --preset <preset-name>
```

Various build presets are defined in `/CMakePresets.json`. To build:

```
cmake --build --preset <preset-name>
```

