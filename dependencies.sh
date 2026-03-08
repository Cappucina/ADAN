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
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        DISTRO="darwin"
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

needs_sudo() {
    if [ "$EUID" -eq 0 ]; then
        SUDO=""
        return 1
    elif command_exists sudo; then
        SUDO="sudo"
        return 0
    else
        print_error "This script requires root privileges or sudo"
        exit 1
    fi
}

install_packages() {
    local packages="$*"

    case $PKG_MGR in
    apt)
        print_info "Updating package list..."
        $SUDO apt-get update -qq || print_warn "Failed to update package list"
        print_info "Installing packages: $packages"
        $SUDO apt-get install -y $packages
        ;;
    dnf)
        print_info "Installing packages: $packages"
        $SUDO dnf install -y $packages
        ;;
    yum)
        print_info "Installing packages: $packages"
        $SUDO yum install -y $packages
        ;;
    pacman)
        print_info "Updating package database..."
        $SUDO pacman -Sy
        print_info "Installing packages: $packages"
        $SUDO pacman -S --noconfirm $packages
        ;;
    zypper)
        print_info "Installing packages: $packages"
        $SUDO zypper install -y $packages
        ;;
    apk)
        print_info "Updating package index..."
        $SUDO apk update
        print_info "Installing packages: $packages"
        $SUDO apk add $packages
        ;;
    brew)
        print_info "Installing packages: $packages"
        brew install $packages
        ;;
    esac
}

get_package_names() {
    case $PKG_MGR in
    apt)
        PACKAGES="build-essential cmake clang-format gdb"
        ;;
    dnf | yum)
        PACKAGES="gcc make cmake clang-tools-extra gdb"
        ;;
    pacman)
        PACKAGES="base-devel cmake clang gdb"
        ;;
    zypper)
        PACKAGES="gcc make cmake clang-tools gdb"
        ;;
    apk)
        PACKAGES="build-base cmake clang-extra-tools gdb"
        ;;
    brew)
        PACKAGES="cmake clang-format"
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
    needs_sudo
    if [ -n "$SUDO" ]; then
        print_info "Will use sudo for package installation"
    fi

    echo
    print_info "Checking current dependencies..."
    echo

    NEED_INSTALL=false
    if [ "$DISTRO" = "darwin" ]; then
        check_dependency "clang" "Clang compiler" || NEED_INSTALL=true
        check_dependency "make" "Make" || NEED_INSTALL=true
        check_dependency "cmake" "CMake" || NEED_INSTALL=true
        check_dependency "clang-format" "clang-format" || NEED_INSTALL=true
        # lldb ships with Xcode Command Line Tools and is not available via brew
        check_dependency "lldb" "LLDB debugger" || print_warn "LLDB is not installed — run: xcode-select --install"
    else
        check_dependency "gcc" "GCC compiler" || NEED_INSTALL=true
        check_dependency "make" "Make" || NEED_INSTALL=true
        check_dependency "cmake" "CMake" || NEED_INSTALL=true
        check_dependency "clang-format" "clang-format" || NEED_INSTALL=true
        check_dependency "gdb" "GDB debugger" || NEED_INSTALL=true
    fi

    echo

    if [ "$NEED_INSTALL" = false ]; then
        print_info "All dependencies are already installed!"
        exit 0
    fi

    get_package_names
    print_info "Installing missing dependencies..."
    echo

    if install_packages $PACKAGES; then
        echo
        print_info "✓ All dependencies installed successfully!"
        echo
        print_info "Verifying installation..."
        echo
        if [ "$DISTRO" = "darwin" ]; then
            check_dependency "clang" "Clang compiler"
            check_dependency "make" "Make"
            check_dependency "cmake" "CMake"
            check_dependency "clang-format" "clang-format"
            check_dependency "lldb" "LLDB debugger" || print_warn "LLDB not found — run: xcode-select --install"
        else
            check_dependency "gcc" "GCC compiler"
            check_dependency "make" "Make"
            check_dependency "cmake" "CMake"
            check_dependency "clang-format" "clang-format"
            check_dependency "gdb" "GDB debugger"
        fi
    else
        print_error "Failed to install some dependencies"
        exit 1
    fi
}

main
