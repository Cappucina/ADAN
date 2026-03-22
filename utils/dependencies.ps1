#!/usr/bin/env pwsh
Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

if (-not ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    Write-Host "[INFO] Restarting as Administrator..." -ForegroundColor Green
    $args = "-NoProfile -ExecutionPolicy Bypass -File `"$PSCommandPath`""
    Start-Process pwsh -Verb RunAs -ArgumentList $args
    exit
}

function Write-Info {
    param([string]$Message)
    Write-Host "[INFO] " -ForegroundColor Green -NoNewline
    Write-Host $Message
}

function Write-Warn {
    param([string]$Message)
    Write-Host "[WARN] " -ForegroundColor Yellow -NoNewline
    Write-Host $Message
}

function Write-Err {
    param([string]$Message)
    Write-Host "[ERROR] " -ForegroundColor Red -NoNewline
    Write-Host $Message
}

function Test-CommandExists {
    param([string]$Command)
    $null -ne (Get-Command $Command -ErrorAction SilentlyContinue)
}

function Get-Platform {
    if ($PSVersionTable.PSVersion.Major -ge 6) {
        if ($IsWindows) { return "windows" }
        if ($IsMacOS)   { return "darwin" }
        if ($IsLinux)   { return "linux" }
    }
    if ($env:OS -eq "Windows_NT") { return "windows" }
    Write-Err "Cannot detect operating system"
    exit 1
}

function Find-PackageManager {
    param([string]$Platform)
    switch ($Platform) {
        "windows" {
            if (Test-CommandExists "choco")  { return "choco" }
            if (Test-CommandExists "scoop")  { return "scoop" }
            if (Test-CommandExists "winget") { return "winget" }
            Write-Err "No supported package manager found (install Chocolatey, Scoop, or winget)"
            exit 1
        }
        "darwin" {
            if (Test-CommandExists "brew") { return "brew" }
            Write-Err "No supported package manager found (install Homebrew)"
            exit 1
        }
        "linux" {
            if (Test-CommandExists "apt-get") { return "apt" }
            if (Test-CommandExists "dnf")     { return "dnf" }
            if (Test-CommandExists "yum")     { return "yum" }
            if (Test-CommandExists "pacman")  { return "pacman" }
            if (Test-CommandExists "zypper")  { return "zypper" }
            if (Test-CommandExists "apk")     { return "apk" }
            Write-Err "No supported package manager found"
            exit 1
        }
    }
}

function Test-IsAdmin {
    if ($script:Platform -eq "windows") {
        $identity  = [Security.Principal.WindowsIdentity]::GetCurrent()
        $principal = [Security.Principal.WindowsPrincipal]$identity
        return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
    }
    return ((& id -u) -eq "0")
}

function Install-DevPackages {
    param([string[]]$Packages)
    switch ($script:PkgMgr) {
        "choco" {
            Write-Info "Installing packages: $($Packages -join ', ')"
            & choco install $Packages -y
            if ($LASTEXITCODE -ne 0) { throw "Chocolatey installation failed" }
        }
        "scoop" {
            foreach ($pkg in $Packages) {
                Write-Info "Installing $pkg..."
                & scoop install $pkg
            }
        }
        "winget" {
            foreach ($pkg in $Packages) {
                Write-Info "Installing $pkg..."
                & winget install --id $pkg -e --silent `
                    --accept-source-agreements --accept-package-agreements
            }
        }
        "brew" {
            Write-Info "Installing packages: $($Packages -join ', ')"
            & brew install $Packages
        }
        "apt" {
            Write-Info "Updating package list..."
            & sudo apt-get update -qq
            Write-Info "Installing packages: $($Packages -join ', ')"
            & sudo apt-get install -y $Packages
        }
        "dnf" {
            Write-Info "Installing packages: $($Packages -join ', ')"
            & sudo dnf install -y $Packages
        }
        "yum" {
            Write-Info "Installing packages: $($Packages -join ', ')"
            & sudo yum install -y $Packages
        }
        "pacman" {
            Write-Info "Updating package database..."
            & sudo pacman -Sy
            Write-Info "Installing packages: $($Packages -join ', ')"
            & sudo pacman -S --noconfirm $Packages
        }
        "zypper" {
            Write-Info "Installing packages: $($Packages -join ', ')"
            & sudo zypper install -y $Packages
        }
        "apk" {
            Write-Info "Updating package index..."
            & sudo apk update
            Write-Info "Installing packages: $($Packages -join ', ')"
            & sudo apk add $Packages
        }
    }
}

function Get-PackageList {
    switch ($script:PkgMgr) {
        "choco"  { return @("mingw", "cmake", "llvm", "ninja", "zig") }
        "scoop"  { return @("gcc", "cmake", "llvm") }
        "winget" { return @("Kitware.CMake", "LLVM.LLVM") }
        "brew"   { return @("cmake", "clang-format") }
        "apt"    { return @("build-essential", "cmake", "clang-format", "gdb") }
        "dnf"    { return @("gcc", "make", "cmake", "clang-tools-extra", "gdb") }
        "yum"    { return @("gcc", "make", "cmake", "clang-tools-extra", "gdb") }
        "pacman" { return @("base-devel", "cmake", "clang", "gdb") }
        "zypper" { return @("gcc", "make", "cmake", "clang-tools", "gdb") }
        "apk"    { return @("build-base", "cmake", "clang-extra-tools", "gdb") }
    }
}

function Test-Dependency {
    param([string]$Command, [string]$DisplayName)
    if (Test-CommandExists $Command) {
        Write-Info ([char]0x2713 + " $DisplayName is installed")
        return $true
    }
    Write-Warn ([char]0x2717 + " $DisplayName is not installed")
    return $false
}

function Test-AllDependencies {
    $allPresent = $true

    switch ($script:Platform) {
        "windows" {
            if ((Test-CommandExists "gcc") -or (Test-CommandExists "cl")) {
                $name = if (Test-CommandExists "gcc") { "GCC" } else { "MSVC" }
                Write-Info ([char]0x2713 + " $name compiler is installed")
            } else {
                Write-Warn ([char]0x2717 + " No C compiler found (gcc / cl)")
                $allPresent = $false
            }

            if ((Test-CommandExists "mingw32-make") -or
                (Test-CommandExists "make") -or
                (Test-CommandExists "nmake")) {
                $tool = if (Test-CommandExists "mingw32-make") { "mingw32-make" }
                        elseif (Test-CommandExists "make")     { "make" }
                        else                                   { "nmake" }
                Write-Info ([char]0x2713 + " $tool is installed")
            } else {
                Write-Warn ([char]0x2717 + " No make tool found (make / mingw32-make / nmake)")
                $allPresent = $false
            }

            if (-not (Test-Dependency "cmake"        "CMake"))        { $allPresent = $false }
            if (-not (Test-Dependency "clang-format"  "clang-format")) { $allPresent = $false }
            if (-not (Test-Dependency "zig"           "Zig compiler")) { $allPresent = $false }
            if (-not (Test-Dependency "ninja"         "Ninja build"))  { $allPresent = $false }
            if (-not (Test-Dependency "gdb"           "GDB debugger")) {
                Write-Warn "GDB not found - use Visual Studio debugger or install MinGW"
            }
        }
        "darwin" {
            if (-not (Test-Dependency "clang"        "Clang compiler")) { $allPresent = $false }
            if (-not (Test-Dependency "make"         "Make"))           { $allPresent = $false }
            if (-not (Test-Dependency "cmake"        "CMake"))          { $allPresent = $false }
            if (-not (Test-Dependency "clang-format" "clang-format"))   { $allPresent = $false }
            if (-not (Test-Dependency "lldb"         "LLDB debugger")) {
                Write-Warn "LLDB is not installed - run: xcode-select --install"
            }
        }
        "linux" {
            if (-not (Test-Dependency "gcc"          "GCC compiler"))  { $allPresent = $false }
            if (-not (Test-Dependency "make"         "Make"))          { $allPresent = $false }
            if (-not (Test-Dependency "cmake"        "CMake"))         { $allPresent = $false }
            if (-not (Test-Dependency "clang-format" "clang-format"))  { $allPresent = $false }
            if (-not (Test-Dependency "gdb"          "GDB debugger"))  { $allPresent = $false }
        }
    }

    return $allPresent
}

function Main {
    $script:Platform = Get-Platform
    $script:PkgMgr  = Find-PackageManager $script:Platform

    Write-Info "Detected platform: $script:Platform"
    Write-Info "Package manager: $script:PkgMgr"

    $isAdmin = Test-IsAdmin
    if ($script:Platform -eq "windows" -and $script:PkgMgr -eq "choco" -and -not $isAdmin) {
        Write-Err "Chocolatey requires administrator privileges. Please run as Administrator."
        exit 1
    }
    if ($isAdmin) {
        Write-Info "Running with elevated privileges"
    }

    Write-Host ""
    Write-Info "Checking current dependencies..."
    Write-Host ""

    if (Test-AllDependencies) {
        Write-Host ""
        Write-Info "All dependencies are already installed!"
        exit 0
    }

    Write-Host ""

    $packages = Get-PackageList
    Write-Info "Installing missing dependencies..."
    Write-Host ""

    try {
        Install-DevPackages $packages

        Write-Host ""
        Write-Info ([char]0x2713 + " All dependencies installed successfully!")
        Write-Host ""
        Write-Info "Verifying installation..."
        Write-Host ""

        if ($script:Platform -eq "windows") {
            $machinePath = [System.Environment]::GetEnvironmentVariable("Path", "Machine")
            $userPath    = [System.Environment]::GetEnvironmentVariable("Path", "User")
            $env:Path    = "$machinePath;$userPath"
        }

        [void](Test-AllDependencies)

        if ($script:PkgMgr -eq "winget") {
            Write-Host ""
            Write-Warn "winget cannot install MinGW (gcc / make / gdb)."
            Write-Warn "Install manually or run: choco install mingw  /  scoop install gcc"
        }
    }
    catch {
        Write-Err "Failed to install some dependencies: $_"
        exit 1
    }
}

Main