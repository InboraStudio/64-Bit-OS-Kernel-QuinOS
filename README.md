# 64-Bit-OS-Kernel-QuinOS PHASE 1: THE BOOTSTRAP
---

## THE MATHEMATICS OF THE KERNEL

Low-level systems programming is the art of translating abstract logic into physical memory addresses. Quin OS utilizes specific mathematical models to manage hardware.

### 1. Framebuffer Address Translation
To render a pixel on a modern display, we must map a 2D coordinate $(x, y)$ into a 1D memory array. The kernel calculates the exact byte offset using the following formula:

$$Address_{pixel} = Base_{VRAM} + (y \cdot Pitch) + (x \cdot \frac{BPP}{8})$$

| Parameter | Function |
| :--- | :--- |
| **Base** | The physical start of the Video RAM provided by the UEFI bootloader. |
| **Pitch** | The total bytes in a horizontal scanline ($Width \times BytesPerPixel$). |
| **BPP** | Bits Per Pixel (Set to 32 for Quin OS RGB format). |



### 2. Higher-Half Direct Map (HHDM) Logic
The kernel exists at the top of the virtual address space. To access a physical address $P$ without remapping page tables constantly, we use a constant offset:

$$V_{target} = P_{physical} + Offset_{HHDM}$$
$$Offset_{HHDM} = 0xFFFF800000000000$$

This allows the kernel to "see" the entire physical RAM as a continuous block starting from the HHDM offset.



### 3. Glyph Rendering Logic (Bitwise Transformation)
Quin OS uses an $8 \times 16$ bitmap font. To render a character $C$, the kernel performs a nested loop over 16 bytes, checking individual bits:

For each row $i \in [0, 15]$ and column $j \in [0, 7]$:
$$\text{Pixel}(x+j, y+i) = \begin{cases} \text{Color}_{FG} & \text{if } (Glyph[i] \gg (7-j)) \& 1 \\ \text{Color}_{BG} & \text{otherwise} \end{cases}$$

---

## SYSTEM ARCHITECTURE

| Component | Specification |
| :--- | :--- |
| **Mode** | 64-bit Long Mode |
| **Address Space** | 48-bit Virtual / 52-bit Physical |
| **Kernel Base** | `0xFFFFFFFF80000000` (-2 GiB) |
| **Page Size** | 4 KiB (Standard) |

### Memory Layout
| Range | Content |
| :--- | :--- |
| `0xFFFFFFFF80000000` | Kernel Code, Data, and Stack |
| `0xFFFF800000000000` | Direct Physical Map (HHDM) |
| `0x0000000000000000` | Reserved for Userland Ring 3 |

---


## Limine Protocol Request/Response

```
Kernel Side:                     Limine Side:
───────────────                  ────────────

struct limine_framebuffer_request {
  .id = MAGIC_NUMBER         ──►  [Limine recognizes request]
  .revision = 0                      │
  .response = NULL                   │
}                                    ▼
                              [Limine initializes framebuffer]
                                     │
                                     ▼
                              [Fills response structure]
                                     │
                                     ▼
.response ◄──────────────────  struct limine_framebuffer_response {
                                .framebuffers[0] = {
                                  .address = 0xFD000000
                                  .width = 1024
                                  .height = 768
                                  .pitch = 4096
                                  .bpp = 32
                                  .memory_model = RGB
                                }
                              }

Similarly for:
- memmap_request   ──►  Memory map entries
- hhdm_request     ──►  HHDM offset
```

## Boot Output

When booting successfully, you should see:

```
  ___        _         ___  ____  
 / _ \ _   _(_)_ __   / _ \/ ___| 
| | | | | | | | '_ \ | | | \___ \ 
| |_| | |_| | | | | || |_| |___) |
 \__\_\\__,_|_|_| |_| \___/|____/ 

Quin OS - Phase 1: Boot
Version 0.1.0
Architecture: x86_64
Bootloader: Limine

=== FRAMEBUFFER INFO ===
Resolution: 1024x768
BPP: 32
Pitch: 4096
Model: RGB
========================

=== MEMORY MAP ===
[Memory regions listed here]
===============

[OK] Kernel initialized successfully
[OK] Entered long mode (x86_64)
[OK] Framebuffer initialized
[OK] Memory map parsed

Phase 1 Complete!
System halted. Press Ctrl+C in QEMU to exit.
```

## DEVELOPMENT ROADMAP

| Phase | Milestone | Objective | Status |
| :--- | :--- | :--- | :--- |
| **1** | **Boot** | UEFI handoff & LFB Mathematics | **ACTIVE** |
| 2 | Memory | PMM/VMM & Heap Allocation | PENDING |
| 3 | Interrupts | IDT, GDT & Exception Handling | PENDING |
| 4 | Scheduling | Preemptive Multi-tasking | PENDING |
| 5 | Linux ABI | System Call Translation Layer | PENDING |

---

## BUILD & EXECUTION

### Toolchain
* **Cross-Compiler:** Clang 17+
* **Linker:** LLD (Script: `linker.ld`)
* **Binary Format:** ELF64 PIE

### Commands
| Target | Action |
| :--- | :--- |
| `make all` | Compiles kernel and generates bootable ISO. |
| `make run` | Launches QEMU with OVMF UEFI firmware. |
| `make clean` | Removes object files and binaries. |

---

## DEBUGGING INTERFACE

The kernel communicates via the serial port (`COM1`) at 115200 baud.
* **Serial Output:** `-serial stdio`
* **GDB:** Port `:1234`

### INBORA STUDIO
**Architect:** Dr Chamyoung  
**License:** MIT  
**Github:** [Inbora Studio](https://github.com/inborastudio)
