# Setup

The following VSCode extensions are required: C/C++ extension pack, clangd, CMake Tools, CMake Language Support, CodeLLDB, CS 128 Clang-Tidy, Git Blame, Git Graph, Git History, VSCode Remote extension pack,
Clang-Format.

## Dependencies

```bash
# Compiler toolchain, make, and Apple's clang/lldb (replaces base-devel)
xcode-select --install

LLVM_VERSION=22
GCC_VERSION=16
brew install cmake ninja ccache git "llvm@${LLVM_VERSION}" "gcc@${GCC_VERSION}"
brew install --cask visual-studio-code
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