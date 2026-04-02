#!/bin/bash
set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

command_exists() {
    command -v "$1" >/dev/null 2>&1
}

detect_distro() {
    if [ "$(uname)" = "Darwin" ]; then
        DISTRO="darwin"
    elif [ -f /etc/os-release ]; then
        . /etc/os-release
        DISTRO=$ID
    elif command_exists lsb_release; then
        DISTRO=$(lsb_release -si | tr '[:upper:]' '[:lower:]')
    else
        print_error "Cannot detect operating system"
        exit 1
    fi

    if command_exists apt-get; then
        PKG_MGR="apt"
    elif command_exists dnf; then
        PKG_MGR="dnf"
    elif command_exists yum; then
        PKG_MGR="yum"
    elif command_exists pacman; then
        PKG_MGR="pacman"
    elif command_exists zypper; then
        PKG_MGR="zypper"
    elif command_exists apk; then
        PKG_MGR="apk"
    elif command_exists brew; then
        PKG_MGR="brew"
    else
        print_error "No supported package manager found"
        exit 1
    fi

    print_info "Detected distribution: $DISTRO"
    print_info "Package manager: $PKG_MGR"
}

set_sudo_command() {
    if [ "$EUID" -eq 0 ]; then
        SUDO=""
        return 0
    elif command_exists sudo; then
        SUDO="sudo"
        return 0
    else
        print_error "This script requires root privileges or sudo"
        exit 1
    fi
}

install_packages() {
    case $PKG_MGR in
    apt)
        print_info "Updating package list..."
        $SUDO apt-get update -qq || print_warn "Failed to update package list; continuing with installation using the existing package cache. If installation fails, please check your network connection or run 'sudo apt-get update' manually and re-run this script."
        print_info "Installing packages: $*"
        $SUDO apt-get install -y "$@"
        ;;
    dnf)
        print_info "Installing packages: $*"
        $SUDO dnf install -y "$@"
        ;;
    yum)
        print_info "Installing packages: $*"
        $SUDO yum install -y "$@"
        ;;
    pacman)
        print_info "Updating package database..."
        $SUDO pacman -Sy
        print_info "Installing packages: $*"
        $SUDO pacman -S --noconfirm "$@"
        ;;
    zypper)
        print_info "Installing packages: $*"
        $SUDO zypper install -y "$@"
        ;;
    apk)
        print_info "Updating package index..."
        $SUDO apk update
        print_info "Installing packages: $*"
        $SUDO apk add "$@"
        ;;
    brew)
        print_info "Installing packages: $*"
        brew install "$@"
        ;;
    esac
}

# Defines the logical set of tools we require across platforms.
# This function is intentionally kept in one place to aid keeping
# dependencies.sh and dependencies.ps1 in sync. Platform-specific
# mappings in get_package_names translate these logical needs into
# concrete package names for each package manager.
get_logical_packages() {
    # Core requirements (conceptual):
    # - C/C++ compiler toolchain (compiler, linker, binutils)
    # - Clang tooling (compiler and extras/formatting where available)
    # - Debugger (gdb)
    # - Build system (xmake)
    #
    # These are expressed as logical capability names. Platform-specific
    # mappings in get_package_names translate them into concrete package
    # names for each package manager.
    LOGICAL_PACKAGES=(
        "c_toolchain"      # C/C++ compiler, linker, binutils/make, etc.
        "clang"            # Clang/LLVM compiler
        "clang_format"     # Clang-format or equivalent formatting tool
        "debugger"         # Debugger (e.g., gdb/lldb)
        "build_system"     # Build system (xmake)
    )
}

get_package_names() {
    # Keep this mapping in sync with the corresponding logic in
    # dependencies.ps1. Any intentional differences between shell and
    # PowerShell versions should be documented inline below.
    get_logical_packages

    case $PKG_MGR in
    apt)
        # apt: build-essential provides gcc/g++, make, and binutils.
        # less is included here for log viewing on Debian/Ubuntu only.
        PACKAGES=(build-essential clang clang-format gdb less binutils xmake libssl-dev)
        ;;
    dnf | yum)
        # dnf/yum: gcc toolchain plus clang and extra tools.
        PACKAGES=(gcc clang clang-tools-extra gdb xmake openssl-devel)
        ;;
    pacman)
        # pacman: base-devel provides build toolchain (including gcc/make).
        PACKAGES=(base-devel clang gdb xmake openssl)
        ;;
    zypper)
        # zypper: gcc toolchain plus clang tools.
        PACKAGES=(gcc clang clang-tools gdb xmake libopenssl-devel)
        ;;
    apk)
        # apk: build-base provides build toolchain, clang-extra-tools adds tooling.
        PACKAGES=(build-base clang-extra-tools gdb xmake openssl-dev)
        ;;
    brew)
        # brew: llvm includes clang; clang-format is separate.
        PACKAGES=(llvm clang-format xmake openssl)
        ;;
    esac
}

check_dependency() {
    local cmd=$1
    local name=$2

    if command_exists "$cmd"; then
        print_info "✓ $name is installed"
        return 0
    else
        print_warn "✗ $name is not installed"
        return 1
    fi
}

main() {
    detect_distro
    set_sudo_command
    if [ -n "$SUDO" ]; then
        print_info "Will use sudo for package installation"
    fi

    echo
    print_info "Checking current dependencies..."
    echo

    NEED_INSTALL=false
    XMAKE_WAS_MISSING=false
    if [ "$DISTRO" = "darwin" ]; then
        check_dependency "clang" "Clang compiler" || NEED_INSTALL=true
        check_dependency "clang-format" "clang-format" || NEED_INSTALL=true
        check_dependency "xmake" "XMake" || { NEED_INSTALL=true; XMAKE_WAS_MISSING=true; }
        check_dependency "openssl" "OpenSSL" || NEED_INSTALL=true
        check_dependency "lldb" "LLDB debugger" || print_warn "LLDB is not installed — run: xcode-select --install"
    else
        check_dependency "gcc" "GCC compiler" || NEED_INSTALL=true
        check_dependency "clang-format" "clang-format" || NEED_INSTALL=true
        check_dependency "xmake" "XMake" || { NEED_INSTALL=true; XMAKE_WAS_MISSING=true; }
        check_dependency "gdb" "GDB debugger" || NEED_INSTALL=true
        check_dependency "openssl" "OpenSSL" || NEED_INSTALL=true
    fi

    echo

    if [ "$NEED_INSTALL" = false ]; then
        print_info "All dependencies are already installed!"
        exit 0
    fi

    get_package_names
    print_info "Installing missing dependencies..."
    echo

    if install_packages "${PACKAGES[@]}"; then
        if [ "$XMAKE_WAS_MISSING" = true ] && [ -n "$GITHUB_PATH" ]; then
            # Detect actual xmake installation path before updating GITHUB_PATH
            XMAKE_PATH="$(command -v xmake || true)"
            if [ -n "$XMAKE_PATH" ]; then
                XMAKE_DIR="$(dirname "$XMAKE_PATH")"
                if [ -d "$XMAKE_DIR" ]; then
                    print_info "Updating GITHUB_PATH with xmake directory: $XMAKE_DIR"
                    echo "$XMAKE_DIR" >> "$GITHUB_PATH"
                else
                    print_warn "Detected xmake directory '$XMAKE_DIR' does not exist; not updating GITHUB_PATH"
                fi
            else
                print_warn "xmake not found in PATH after installation; not updating GITHUB_PATH"
            fi
        fi
        echo
        print_info "✓ All dependencies installed successfully!"
        echo
        print_info "Verifying installation..."
        echo
        if [ "$DISTRO" = "darwin" ]; then
            check_dependency "clang" "Clang compiler"
            check_dependency "clang-format" "clang-format"
            check_dependency "xmake" "XMake"
            check_dependency "lldb" "LLDB debugger" || print_warn "LLDB not found — run: xcode-select --install"
        else
            check_dependency "gcc" "GCC compiler"
            check_dependency "clang-format" "clang-format"
            check_dependency "xmake" "XMake"
            check_dependency "gdb" "GDB debugger"
        fi
    else
        print_error "Failed to install some dependencies"
        exit 1
    fi
}

main
