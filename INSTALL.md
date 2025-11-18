# ADAN Installation Guide

## Quick Installation

### Arch Linux / Manjaro (via makepkg)

```bash
# Build and install from source
makepkg -si

# Or install from AUR (once published)
paru -S adan
# or
yay -S adan
```

### Debian / Ubuntu

```bash
# Install dependencies (once published)
sudo apt-get install adan

# Uninstall
sudo make uninstall
```

### macOS (Homebrew)

```bash
# Install via Homebrew tap
brew tap placeholder/adan
brew install adan
```

### From Source (Any Linux/Unix)

```bash
# Install dependencies
# For Arch/Manjaro:
sudo pacman -S rust nasm gcc

# For Debian/Ubuntu:
sudo apt-get install cargo rustc nasm gcc

# For Fedora:
sudo dnf install cargo rust nasm gcc

# For Mac:
brew install cargo rust nasm
sudo xcode-select --install

# Build and install
make release
sudo make install

# Or install to user directory (no sudo required)
make install PREFIX=~/.local
```

## Installation Paths

### System-wide Installation
- **Binary**: `/usr/local/bin/adan`
- **Helper**: `/usr/local/bin/adan-build-asm`
- **Documentation**: `/usr/local/share/doc/adan/`
- **Examples**: `/usr/local/share/adan/examples/`

### User Installation (`PREFIX=~/.local`)
- **Binary**: `~/.local/bin/adan`
- **Helper**: `~/.local/bin/adan-build-asm`
- **Documentation**: `~/.local/share/doc/adan/`
- **Examples**: `~/.local/share/adan/examples/`

Make sure `~/.local/bin` is in your PATH:
```bash
echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc
```

## Custom Installation

You can customize the installation prefix:

```bash
# Install to /opt/adan
sudo make install PREFIX=/opt/adan

# Install to custom directory
make install PREFIX=/custom/path DESTDIR=/staging/area
```

## Verification

After installation, verify it works:

```bash
# Check version
adan --help

# Try an example
adan --native /usr/local/share/adan/examples/Math.adn
./Math
```

## Uninstallation

```bash
# System-wide
sudo make uninstall

# User installation
make uninstall PREFIX=~/.local
```

## Building Packages

### Arch Linux Package

```bash
makepkg -s
sudo pacman -U adan-0.1.0-1-x86_64.pkg.tar.zst
```

### Debian Package

```bash
dpkg-buildpackage -us -uc
sudo dpkg -i ../adan_0.1.0_amd64.deb
```

### Distribution Tarball

```bash
make dist
# Creates dist/adan-0.1.0.tar.gz
```

## Dependencies

### Runtime Dependencies
- `gcc` or `clang` - For linking native executables
- `nasm` - For assembling x86-64 assembly code

### Build Dependencies
- `rust` (rustc) - Rust compiler
- `cargo` - Rust package manager
- `nasm` - NASM assembler
- `gcc` or `clang` - C compiler

## Troubleshooting

### Missing dependencies
```bash
# Arch Linux
sudo pacman -S rust nasm gcc

# Debian/Ubuntu
sudo apt-get install cargo rustc nasm gcc

# macOS
brew install rust nasm
```

### Permission denied during installation
Use `sudo` for system-wide installation or install to user directory:
```bash
make install PREFIX=~/.local
```

### Binary not found after installation
Make sure the installation directory is in your PATH:
```bash
# For system-wide installation
echo $PATH | grep -q "/usr/local/bin" || echo 'export PATH="/usr/local/bin:$PATH"' >> ~/.bashrc

# For user installation
echo $PATH | grep -q "$HOME/.local/bin" || echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc
```

## Support

For issues, please visit: https://github.com/yourusername/ADAN/issues
