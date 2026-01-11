# Quin OS - Root Makefile

.PHONY: all kernel iso run clean

# Paths
KERNEL_DIR := kernel
ISO_DIR := iso
LIMINE_DIR := limine

# Output files
ISO_FILE := quin-os.iso
KERNEL_ELF := $(KERNEL_DIR)/quinkernel.elf

all: iso

# Build kernel
kernel:
	@echo "=== Building Kernel ==="
	@$(MAKE) -C $(KERNEL_DIR)

# Create bootable ISO
iso: kernel
	@echo "=== Creating Bootable ISO ==="
	@rm -rf $(ISO_DIR)
	@mkdir -p $(ISO_DIR)/boot
	@mkdir -p $(ISO_DIR)/boot/limine
	@mkdir -p $(ISO_DIR)/EFI/BOOT
	
	# Copy kernel
	@cp $(KERNEL_ELF) $(ISO_DIR)/boot/
	
	# Copy Limine bootloader files
	@cp $(LIMINE_DIR)/limine-bios.sys $(ISO_DIR)/boot/limine/ 2>/dev/null || true
	@cp $(LIMINE_DIR)/limine-bios-cd.bin $(ISO_DIR)/boot/limine/ 2>/dev/null || true
	@cp $(LIMINE_DIR)/limine-uefi-cd.bin $(ISO_DIR)/boot/limine/ 2>/dev/null || true
	@cp $(LIMINE_DIR)/BOOTX64.EFI $(ISO_DIR)/EFI/BOOT/ 2>/dev/null || true
	@cp $(LIMINE_DIR)/BOOTIA32.EFI $(ISO_DIR)/EFI/BOOT/ 2>/dev/null || true
	
	# Copy Limine config
	@cp $(LIMINE_DIR)/limine.conf $(ISO_DIR)/boot/limine/
	
	# Create ISO
	@xorriso -as mkisofs \
		-b boot/limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		$(ISO_DIR) -o $(ISO_FILE)
	
	# Install Limine on ISO (for BIOS boot)
	@$(LIMINE_DIR)/limine bios-install $(ISO_FILE) 2>/dev/null || true
	
	@echo "ISO created: $(ISO_FILE)"

# Run in QEMU
run: iso
	@echo "=== Launching QEMU ==="
	@if [ -f "$(LIMINE_DIR)/OVMF.fd" ]; then \
		qemu-system-x86_64 \
			-M q35 \
			-m 256M \
			-bios $(LIMINE_DIR)/OVMF.fd \
			-cdrom "$(shell pwd)/$(ISO_FILE)" \
			-serial stdio \
			-d int,cpu_reset \
			-no-reboot \
			-no-shutdown; \
	else \
		echo "OVMF.fd not found, trying QEMU built-in UEFI..."; \
		qemu-system-x86_64 \
			-M q35 \
			-m 256M \
			-drive if=pflash,format=raw,readonly=on,file=/mingw64/share/edk2-x86_64-code.fd \
			-cdrom "$(shell pwd)/$(ISO_FILE)" \
			-serial stdio \
			-d int,cpu_reset \
			-no-reboot \
			-no-shutdown; \
	fi

# Clean all build artifacts
clean:
	@$(MAKE) -C $(KERNEL_DIR) clean
	@rm -rf $(ISO_DIR) $(ISO_FILE)
	@echo "Cleaned all build artifacts"
