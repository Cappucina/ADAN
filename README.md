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
$ xmake install       # Install remaining dependencies (Clang, LLVM, etc.)
$ xmake               # Build the ADAN compiler
$ xmake run           # Compile the default sample with the built compiler
$ ./samples/testing
```

## XMake Commands
```bash
$ xmake               # Build the ADAN compiler.
$ xmake run           # Run the compiler with the default sample arguments.
$ ./samples/testing   # Run the generated testing sample executable.
$ xmake format        # Format all source code using clang-format.
$ xmake install       # Install all required system dependencies.
$ xmake c             # Remove all build artifacts.
$ xmake f -a x64      # Configure for specific architecture (x64, arm64, etc.)
```

`xmake run` invokes the `adan` target with these default arguments:

```bash
$ ./build/linux/x86_64/release/adan -f samples/testing.adn -o samples/testing
```

This generates `samples/testing` on Linux and macOS or `samples/testing.exe` on Windows.

## Compiler Arguments

ADAN's compiler provides flags to modify default compilation behaviors:

- `-f <file>` / `--file <file>`: Source file to compile (`.adn` or `.adan`). This argument is required.
- `-o <path>` / `--output <path>`: Specifies the output executable path. If `<path>` is a directory, the binary is placed inside it named after the source file. Defaults to the input source file name in the current directory if not provided.
- `-r` / `--rawir`: Emits the LLVM IR (`.ll` file) instead of compiling into an executable binary.
- `-l <name>` / `--link-lib <name>`: Adds a native library to the final linker invocation.
- `-L <path>` / `--link-search <path>`: Adds a native library search path to the final linker invocation.
- `--link-arg <arg>`: Passes a raw argument directly to the system linker.
- `-h` / `--help`: Show the help message and exit.

```powershell
$ ./adan -f main.adn -r    # Output 'main.ll' LLVM IR.
$ ./adan --file main.adn   # Outputs a 'main' executable.
```

## Native Linking

ADAN now supports source-level native-linking metadata for FFI declarations and top-level linker directives:

```adan
link "m";
link_search "/usr/lib";

extern function puts(text: string): i32 link "puts" library "c" abi "c";
```

You can also export a definition from ADAN for the generated object code:

```adan
export function add(a: i32, b: i32): i32 {
  return a + b;
}
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
