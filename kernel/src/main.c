#include <stdint.h>
#include <stddef.h>
#include "limine.h"
#include "framebuffer.h"

/* Halt the CPU */
static void hcf(void) {
    for (;;) {
        __asm__ volatile ("cli; hlt");
    }
}

/* Panic handler */
__attribute__((noreturn))
void panic(const char *message) {
    fb_puts_color("\n\n=== KERNEL PANIC ===\n", 0x00FF0000, 0x00000000);
    fb_puts_color(message, 0x00FFFFFF, 0x00000000);
    fb_puts_color("\n===================\n", 0x00FF0000, 0x00000000);
    hcf();
}

/* Set the base revision to 2 (Limine boot protocol) */
LIMINE_BASE_REVISION(2)

/* Framebuffer request */
__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0,
    .response = NULL
};

/* Memory map request */
__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
    .response = NULL
};

/* HHDM request (Higher Half Direct Map) */
__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0,
    .response = NULL
};

/* Halt and catch fire marker for requests */
__attribute__((used, section(".limine_requests_start_marker")))
static volatile uint64_t limine_requests_start_marker[4] = {
    0xf6b8f4b39de7d1ae, 0xfab91a6940fcb9cf,
    0x785c6ed015d3e316, 0x181e920a7852b9d9
};

__attribute__((used, section(".limine_requests_end_marker")))
static volatile uint64_t limine_requests_end_marker[2] = {
    0xadc0e0531bb10d03, 0x9572709f31764c62
};

/* Helper to convert number to hex string */
static void utoa_hex(uint64_t value, char *buf, int width) {
    const char hex[] = "0123456789ABCDEF";
    for (int i = width - 1; i >= 0; i--) {
        buf[i] = hex[value & 0xF];
        value >>= 4;
    }
    buf[width] = '\0';
}

/* Print memory map */
static void print_memory_map(void) {
    if (!memmap_request.response) {
        fb_puts("No memory map available\n");
        return;
    }
    
    struct limine_memmap_response *response = (struct limine_memmap_response *)memmap_request.response;
    
    fb_puts_color("\n=== MEMORY MAP ===\n", 0x0000FFFF, 0x00000000);
    
    char buf[32];
    for (uint64_t i = 0; i < response->entry_count; i++) {
        struct limine_memmap_entry *entry = response->entries[i];
        
        fb_puts("Base: 0x");
        utoa_hex(entry->base, buf, 16);
        fb_puts(buf);
        
        fb_puts(" | Length: 0x");
        utoa_hex(entry->length, buf, 16);
        fb_puts(buf);
        
        fb_puts(" | Type: ");
        switch (entry->type) {
            case LIMINE_MEMMAP_USABLE:
                fb_puts_color("USABLE", 0x0000FF00, 0x00000000);
                break;
            case LIMINE_MEMMAP_RESERVED:
                fb_puts("RESERVED");
                break;
            case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
                fb_puts("ACPI_RECLAIMABLE");
                break;
            case LIMINE_MEMMAP_ACPI_NVS:
                fb_puts("ACPI_NVS");
                break;
            case LIMINE_MEMMAP_BAD_MEMORY:
                fb_puts_color("BAD_MEMORY", 0x00FF0000, 0x00000000);
                break;
            case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
                fb_puts("BOOTLOADER_RECLAIMABLE");
                break;
            case LIMINE_MEMMAP_KERNEL_AND_MODULES:
                fb_puts("KERNEL_AND_MODULES");
                break;
            case LIMINE_MEMMAP_FRAMEBUFFER:
                fb_puts("FRAMEBUFFER");
                break;
            default:
                fb_puts("UNKNOWN");
        }
        
        fb_puts("\n");
    }
    
    fb_puts_color("==================\n\n", 0x0000FFFF, 0x00000000);
}

/* Kernel entry point */
void _start(void) {
    /* Check if framebuffer request was fulfilled */
    if (framebuffer_request.response == NULL || 
        framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }
    
    /* Get the first framebuffer */
    struct limine_framebuffer_response *fb_response = 
        (struct limine_framebuffer_response *)framebuffer_request.response;
    struct limine_framebuffer *framebuffer = fb_response->framebuffers[0];
    
    /* Initialize framebuffer */
    fb_init(framebuffer);
    
    /* Clear screen to black */
    fb_clear(0x00000000);
    
    /* Display boot banner */
    fb_puts_color("\n", 0x00FFFFFF, 0x00000000);
    fb_puts_color("  ___        _         ___  ____  \n", 0x00FF00FF, 0x00000000);
    fb_puts_color(" / _ \\ _   _(_)_ __   / _ \\/ ___| \n", 0x00FF00FF, 0x00000000);
    fb_puts_color("| | | | | | | | '_ \\ | | | \\___ \\ \n", 0x00FF00FF, 0x00000000);
    fb_puts_color("| |_| | |_| | | | | || |_| |___) |\n", 0x00FF00FF, 0x00000000);
    fb_puts_color(" \\__\\_\\\\__,_|_|_| |_| \\___/|____/ \n", 0x00FF00FF, 0x00000000);
    fb_puts_color("\n", 0x00FFFFFF, 0x00000000);
    
    fb_puts_color("Quin OS - Phase 1: Boot\n", 0x00FFFF00, 0x00000000);
    fb_puts_color("Version 0.1.0\n", 0x00808080, 0x00000000);
    fb_puts_color("Architecture: x86_64\n", 0x00808080, 0x00000000);
    fb_puts_color("Bootloader: Limine\n\n", 0x00808080, 0x00000000);
    
    /* Print framebuffer info */
    fb_puts_color("=== FRAMEBUFFER INFO ===\n", 0x0000FFFF, 0x00000000);
    
    char buf[32];
    fb_puts("Resolution: ");
    utoa_hex(framebuffer->width, buf, 4);
    fb_puts(buf);
    fb_puts("x");
    utoa_hex(framebuffer->height, buf, 4);
    fb_puts(buf);
    fb_puts("\n");
    
    fb_puts("BPP: ");
    utoa_hex(framebuffer->bpp, buf, 2);
    fb_puts(buf);
    fb_puts("\n");
    
    fb_puts("Pitch: ");
    utoa_hex(framebuffer->pitch, buf, 4);
    fb_puts(buf);
    fb_puts("\n");
    
    fb_puts("Model: ");
    if (framebuffer->memory_model == LIMINE_FRAMEBUFFER_RGB) {
        fb_puts_color("RGB\n", 0x0000FF00, 0x00000000);
    } else {
        fb_puts("Unknown\n");
    }
    
    fb_puts_color("========================\n", 0x0000FFFF, 0x00000000);
    
    /* Print memory map */
    print_memory_map();
    
    /* Print HHDM offset if available */
    if (hhdm_request.response) {
        fb_puts_color("=== HIGHER HALF DIRECT MAP ===\n", 0x0000FFFF, 0x00000000);
        fb_puts("HHDM Offset: 0x");
        utoa_hex(((struct limine_hhdm_response *)hhdm_request.response)->offset, buf, 16);
        fb_puts(buf);
        fb_puts("\n");
        fb_puts_color("===============================\n\n", 0x0000FFFF, 0x00000000);
    }
    
    /* Success message */
    fb_puts_color("[OK] ", 0x0000FF00, 0x00000000);
    fb_puts("Kernel initialized successfully\n");
    
    fb_puts_color("[OK] ", 0x0000FF00, 0x00000000);
    fb_puts("Entered long mode (x86_64)\n");
    
    fb_puts_color("[OK] ", 0x0000FF00, 0x00000000);
    fb_puts("Framebuffer initialized\n");
    
    fb_puts_color("[OK] ", 0x0000FF00, 0x00000000);
    fb_puts("Memory map parsed\n\n");
    
    fb_puts_color("Phase 1 Complete!\n", 0x00FFFF00, 0x00000000);
    fb_puts("System halted. Press Ctrl+C in QEMU to exit.\n");
    
    /* Halt */
    hcf();
}
