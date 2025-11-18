#!/bin/bash
# Cross-platform assembly build script for ADAN compiler output

set -e

# Detect operating system
OS="$(uname -s)"
ASM_FILE="${1:-Math.asm}"
OUTPUT_NAME="${2:-output}"

# Get base name without extension
BASENAME=$(basename "$ASM_FILE" .asm)

case "$OS" in
    Darwin)
        # macOS
        echo "Building for macOS..."
        nasm -f macho64 "$ASM_FILE" -o "${BASENAME}.o"
        ld -macosx_version_min 10.7.0 -lSystem -o "$OUTPUT_NAME" "${BASENAME}.o"
        ;;
    Linux)
        # Linux - need to convert macOS-style symbols to Linux-style
        echo "Building for Linux..."

        # Create a temporary file with Linux-compatible symbols
        TEMP_FILE="${BASENAME}_linux.asm"

        # Convert underscore-prefixed symbols to Linux format
        sed 's/_main/main/g; s/_printf/printf/g; s/_exit/exit/g; s/_strlen/strlen/g; s/_malloc/malloc/g; s/_strcpy/strcpy/g; s/_strcat/strcat/g' "$ASM_FILE" > "$TEMP_FILE"

        # Assemble
        nasm -f elf64 "$TEMP_FILE" -o "${BASENAME}.o"

        # Link with gcc (works on both Linux and macOS)
        if command -v gcc &> /dev/null; then
            gcc -no-pie "${BASENAME}.o" -o "$OUTPUT_NAME"
        elif command -v clang &> /dev/null; then
            clang -no-pie "${BASENAME}.o" -o "$OUTPUT_NAME"
        else
            echo "Error: Neither gcc nor clang found!"
            exit 1
        fi

        # Clean up temp file
        rm -f "$TEMP_FILE"
        ;;
    *)
        echo "Unsupported operating system: $OS"
        exit 1
        ;;
esac

echo "✓ Build complete: $OUTPUT_NAME"
