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
Before compiling the compiler you need to have each dependency installed first.

> [!NOTE]
> This is only required if you plan on compiling the compiler manually instead of using the pre-compiled compiler binary.

```powershell
$ chmod +x ./dependencies.sh
$ ./dependencies.sh
```

Compiler ADAN's binary file using `make`.
```powershell
$ make
```

## Different Make Commands
```powershell
$ make                     # Clean, build, and run the binary file.
$ make build               # Clean and create a fresh binary.
$ make emit                # Build and emit LLVM IR for the sample file.
$ make link                # Build, compile, and link the sample file.
$ make run                 # Clear the terminal, then run the sample binary.
$ make format              # Beautifies all C and header files in ./src and ./libs, using .clang-format.
$ make clean               # Removes all build artifacts and sample outputs.
$ make install             # Install all required dependencies. (Linux required for now!)
$ make build-macos-arm64   # Build the binary for macOS ARM64 (Apple Silicon).
$ make build-macos-x86_64  # Build the binary for macOS x86_64 (Intel Macs).
$ make build-macos         # Build both macOS binaries (ARM64 and x86_64).
$ make push                # Run the push.sh script (for maintainers).
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
