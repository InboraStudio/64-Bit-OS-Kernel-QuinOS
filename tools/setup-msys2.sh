#!/bin/bash

# Quin OS - MSYS2 Development Environment Setup
# Run this in MSYS2 MINGW64 shell

set -e

echo "=== Quin OS MSYS2 Setup ==="

# Update package database
echo "[1/5] Updating package database..."
pacman -Syu --noconfirm

# Install compiler toolchain
echo "[2/5] Installing LLVM/Clang toolchain..."
pacman -S --noconfirm \
    mingw-w64-x86_64-clang \
    mingw-w64-x86_64-lld \
    mingw-w64-x86_64-llvm \
    nasm \
    make

# Install build tools
echo "[3/5] Installing build utilities..."
pacman -S --noconfirm \
    mtools \
    xorriso \
    git \
    wget \
    unzip

# Install QEMU
echo "[4/5] Installing QEMU..."
pacman -S --noconfirm mingw-w64-x86_64-qemu

# Download Limine bootloader
echo "[5/5] Downloading Limine bootloader..."
cd "$(dirname "$0")/.."
mkdir -p limine
cd limine

LIMINE_VERSION="8.4.3"
if [ ! -f "limine-uefi-cd.bin" ]; then
    echo "Downloading Limine ${LIMINE_VERSION}..."
    wget -q "https://github.com/limine-bootloader/limine/releases/download/v${LIMINE_VERSION}/limine-${LIMINE_VERSION}.tar.gz"
    tar -xzf "limine-${LIMINE_VERSION}.tar.gz"
    mv "limine-${LIMINE_VERSION}"/* .
    rm -rf "limine-${LIMINE_VERSION}" "limine-${LIMINE_VERSION}.tar.gz"
fi

# Download OVMF UEFI firmware
echo "Downloading OVMF UEFI firmware..."
if [ ! -f "OVMF.fd" ]; then
    wget -q "https://github.com/osdev0/edk2-ovmf-nightly/releases/latest/download/ovmf-x64.zip"
    unzip -q ovmf-x64.zip
    mv ovmf-x64/OVMF.fd .
    rm -rf ovmf-x64 ovmf-x64.zip
fi

cd ..

echo ""
echo "=== Setup Complete! ==="
echo ""
echo "Toolchain installed:"
echo "  - Clang: $(clang --version | head -n1)"
echo "  - NASM: $(nasm -v)"
echo "  - QEMU: $(qemu-system-x86_64 --version | head -n1)"
echo ""
echo "Next steps:"
echo "  make all    # Build kernel"
echo "  make run    # Run in QEMU"
