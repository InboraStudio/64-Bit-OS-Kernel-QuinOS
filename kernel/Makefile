# Quin OS Kernel Makefile

# Toolchain
CC := clang
AR := llvm-ar

# Kernel source files
KERNEL_C_SOURCES := $(wildcard src/*.c)
KERNEL_OBJECTS := $(KERNEL_C_SOURCES:.c=.o)

# Compiler flags
CFLAGS := \
    -target x86_64-unknown-none-elf \
    -mcmodel=kernel \
    -ffreestanding \
    -fno-stack-protector \
    -fno-stack-check \
    -fno-pic \
    -fno-pie \
    -m64 \
    -march=x86-64 \
    -mno-red-zone \
    -mno-80387 \
    -mno-mmx \
    -mno-sse \
    -mno-sse2 \
    -Wall \
    -Wextra \
    -std=c11 \
    -O2

# Linker script
LINKER_SCRIPT := linker.ld

# Output kernel binary
KERNEL := quinkernel.elf

.PHONY: all clean

all: $(KERNEL)

# Link kernel using clang
$(KERNEL): $(KERNEL_OBJECTS) $(LINKER_SCRIPT)
	@echo "[LD] $@"
	@/mingw64/bin/lld -flavor gnu -m elf_x86_64 -T $(LINKER_SCRIPT) -o $@ $(KERNEL_OBJECTS)

# Compile C source files
%.o: %.c
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	@rm -f $(KERNEL_OBJECTS) $(KERNEL)
	@echo "Cleaned kernel build artifacts"
