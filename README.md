<div align="center">
	<h1>The ADAN Programming Language</h1>
	<p>Writing safe and reliable reusable code to make things a whole lot easier.</p>
</div>


ADAN is a statically typed, memory safe programming language that strives to introduce a strict and safe type system that primarily focuses on preventing possible memory leaks and unstable code. ADAN has syntax similar to C, to keep familiarity and to avoid having a steep learning curve.

ADAN was developed by [Lily](https://github.com/transicle), [Kauht](https://github.com/kauht), and [Leo](https://github.com/nvttles).

> [!WARNING]
> This is my ([Lily](https://github.com/transicle)) very first C project, and my second compiler ever written. Optimization to the project will be made over time and as the project expands and more features get added. My previous compiler was written in Rust, but I stopped to try and have fun with manual memory management.

The ADAN language is open for contributions. If you have found a bug and you would like to fix it, or you would like to make optimizations to our code, open a pull request, and it will be reviewed as soon as possible.
> [Learn how to make a contribution to ADAN.](https://github.com/Cappucina/ADAN/blob/main/CONTRIBUTING.md)


## Installing ADAN

Since **November 20th**, **2025**, ADAN is not available for installation. Public releases may come in the near future.


## Building and running

To build and run the project using Docker, use:

```sh
make compile
make execute
```

### Apple Silicon (M1/M2)

On Apple Silicon machines Docker may build an arm64 image by default. The compiler back-end currently emits x86_64 assembly (e.g. `pushq`, `movq`), so assembling that inside an arm64 container fails with "unknown mnemonic" errors. The project's container build now forces an amd64 image when possible so `make compile` / `make execute` work on Apple Silicon:

- `scripts/container.py` uses `docker build --platform linux/amd64` (falling back to a normal build when `--platform` isn't supported).
- If you still have trouble, ensure Docker buildx is available or explicitly build the image on an x86_64 machine.

### String literal escape sequences

String literals now support common escape sequences in source code, for example:

- `"Hello\nWorld"` becomes a string with an actual newline between `Hello` and `World` at runtime.
- Supported escapes: `\n`, `\t`, `\r`, `\\`, `\"`, `\'`, and `\0`.
