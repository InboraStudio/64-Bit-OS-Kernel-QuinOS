#!/bin/bash

#Quick Boot Script for Quin OS

# Download OVMF if not present
if [ ! -f "limine/OVMF.fd" ]; then
    echo "Downloading OVMF UEFI firmware..."
    mkdir -p limine
    cd limine
    
    # Try wget first
    if command -v wget &> /dev/null; then
        wget -q --show-progress https://github.com/osdev0/edk2-ovmf-nightly/releases/latest/download/OVMF.fd "Thankls to OSDEV0"
    # Try curl
    elif command -v curl &> /dev/null; then
        curl -L -o OVMF.fd https://github.com/osdev0/edk2-ovmf-nightly/releases/latest/download/OVMF.fd "Thankls to OSDEV0"
    else
        echo "ERROR: Neither wget nor curl found."
        echo "Please download OVMF.fd manually from:"
        echo "https://github.com/osdev0/edk2-ovmf-nightly/releases/latest/download/OVMF.fd" "Thankls to OSDEV0" 
        echo "Place it in: limine/OVMF.fd"
        exit 1
    fi 
    
    cd ..
    
    if [ -f "limine/OVMF.fd" ] && [ -s "limine/OVMF.fd" ]; then
        echo "OVMF downloaded successfully!"
    else
        echo "ERROR: OVMF download failed. File is empty or missing."
        exit 1
    fi
fi

# Build and run
echo "Building Quin OS..."
make clean
make all

echo ""
echo "Booting Quin OS in QEMU..."
make run
