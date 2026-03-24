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

set_sudo_command() {
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

    case $PKG_MGR in
    apt)
        print_info "Updating package list..."
        $SUDO apt-get update -qq || print_warn "Failed to update package list"
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

get_package_names() {
    case $PKG_MGR in
    apt)
        PACKAGES="build-essential clang clang-format gdb less binutils xmake"
        ;;
    dnf | yum)
        PACKAGES="gcc clang clang-tools-extra gdb xmake"
        ;;
    pacman)
        PACKAGES="base-devel clang gdb xmake"
        ;;
    zypper)
        PACKAGES="gcc clang clang-tools gdb xmake"
        ;;
    apk)
        PACKAGES="build-base clang-extra-tools gdb xmake"
        ;;
    brew)
        PACKAGES="clang clang-format xmake"
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
    if [ "$DISTRO" = "darwin" ]; then
        check_dependency "clang" "Clang compiler" || NEED_INSTALL=true
        check_dependency "clang-format" "clang-format" || NEED_INSTALL=true
        check_dependency "xmake" "XMake" || NEED_INSTALL=true
        check_dependency "lldb" "LLDB debugger" || print_warn "LLDB is not installed — run: xcode-select --install"
    else
        check_dependency "gcc" "GCC compiler" || NEED_INSTALL=true
        check_dependency "clang-format" "clang-format" || NEED_INSTALL=true
        check_dependency "xmake" "XMake" || NEED_INSTALL=true
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
