<div align="center">
    <h1>The ADAN Programming Language</h1>
    <img width="180" height="180" align="right" src="https://github.com/user-attachments/assets/ba158dc8-846e-4ea6-9aab-f8bd1eee6e01" />
    <p align="left">
        A modern, memory safe programming language designed to be written like JavaScript and used like C for low-level programming.
    </p>
</div>

<p align="center">
 ADAN has been continuously maintained from between <b>February 4th, 2026</b> to <b>Now</b>.
</p>

<div align="center">
  <img src="https://github.com/Cappucina/ADAN/actions/workflows/build.yml/badge.svg" alt="Build Status"/>

  <img src="https://img.shields.io/github/v/release/Cappucina/ADAN?style=flat&label=Releases&color=blue" alt="Releases"/>
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
$ make         # Clean, build, and run the binary file.
$ make build   # Clean and create a fresh binary.
$ make run     # Clear the terminal, then run an existing binary.
$ make format  # Beautifies all C files in the `./src` dir, respecting rules from `.clang-format`.
$ make clean   # Removes an existing binary in `./build`
$ make install # Install all required dependencies. (Linux required for now!)
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
