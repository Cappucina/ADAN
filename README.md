<div align="center">
    <h1>The ADAN Programming Language</h1>
    <img align="right" width="180" height="180" alt="image" src="https://github.com/user-attachments/assets/d0a04208-84b8-405a-ac47-73960431d5d6" />
    <p align="left">
        A modern, memory safe programming language designed to be written like JavaScript and used like C for low-level programming.
    </p>
</div>

<p align="left">
 ADAN has been continuously maintained from between <b>February 4th, 2026</b> to <b>Now</b>.
</p>

<div align="center">
  <img src="https://github.com/Cappucina/ADAN/actions/workflows/build.yml/badge.svg" alt="Build Status"/>

  <a href="https://github.com/Cappucina/ADAN/releases">
    <img src="https://img.shields.io/github/v/release/Cappucina/ADAN?style=flat&label=Releases&color=blue" alt="Releases"/>
  </a>
</div>

---

## Compiling the Compiler
Before compiling the compiler, you need to have [XMake](https://xmake.io/) installed.

> [!NOTE]
> Manual compilation is only required if you prefer not to use the pre-compiled binaries available in [Releases](https://github.com/Cappucina/ADAN/releases).

```bash
$ xmake install   # Install remaining dependencies (Clang, LLVM, etc.)
$ xmake           # Build and run the default sample in one go
```

## XMake Commands
```bash
$ xmake            # Build and run the default testing sample.
$ xmake run        # Rebuild (if needed) and run the testing sample.
$ xmake format     # Format all source code using clang-format.
$ xmake install    # Install all required system dependencies.
$ xmake c          # Remove all build artifacts.
$ xmake f -a x64   # Configure for specific architecture (x64, arm64, etc.)
```

## Compiler Arguments

ADAN's compiler provides flags to modify default compilation behaviors:

- `-f <file>` / `--file <file>`: Source file to compile (`.adn` or `.adan`). This argument is required.
- `-o <path>` / `--output <path>`: Specifies the output executable path. If `<path>` is a directory, the binary is placed inside it named after the source file. Defaults to the input source file name in the current directory if not provided.
- `-r` / `--rawir`: Emits the LLVM IR (`.ll` file) instead of compiling into an executable binary.
- `-h` / `--help`: Show the help message and exit.

```powershell
$ ./adan -f main.adn -r        # Output 'main.ll' LLVM IR.
$ ./adan --file main.adn       # Outputs a 'main' executable.
```

<br>
<div align="center">
  <table>
    <tr>
      <td>
        <picture>
          <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/svg?repos=Cappucina/ADAN&type=Date&theme=dark" />
          <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/svg?repos=Cappucina/ADAN&type=Date" />
          <img alt="Star History Chart" src="https://api.star-history.com/svg?repos=Cappucina/ADAN&type=Date" width="600" />
        </picture>
      </td>
    </tr>
  </table>
</div>
