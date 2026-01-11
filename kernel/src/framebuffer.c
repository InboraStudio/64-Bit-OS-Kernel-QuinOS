#include "framebuffer.h"
#include "font.h"

/* Framebuffer state */
static struct limine_framebuffer *fb = NULL;
static uint64_t cursor_x = 0;
static uint64_t cursor_y = 0;
static uint32_t fg_color = COLOR_WHITE;
static uint32_t bg_color = COLOR_BLACK;

/* Memory functions (freestanding C) */
static void *memset(void *s, int c, size_t n) {
    unsigned char *p = (unsigned char *)s;
    while (n--) {
        *p++ = (unsigned char)c;
    }
    return s;
}

static void *memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

static size_t strlen(const char *str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

/* Initialize framebuffer */
void fb_init(struct limine_framebuffer *framebuffer) {
    fb = framebuffer;
    cursor_x = 0;
    cursor_y = 0;
    fg_color = COLOR_WHITE;
    bg_color = COLOR_BLACK;
}

/* Clear screen with color */
void fb_clear(uint32_t color) {
    if (!fb) return;
    
    uint32_t *pixels = (uint32_t *)fb->address;
    uint64_t pixel_count = fb->width * fb->height;
    
    for (uint64_t i = 0; i < pixel_count; i++) {
        pixels[i] = color;
    }
    
    cursor_x = 0;
    cursor_y = 0;
}

/* Set pixel at coordinates */
void fb_putpixel(uint64_t x, uint64_t y, uint32_t color) {
    if (!fb || x >= fb->width || y >= fb->height) return;
    
    uint32_t *pixels = (uint32_t *)fb->address;
    pixels[y * fb->width + x] = color;
}

/* Scroll screen up by one line */
static void fb_scroll(void) {
    if (!fb) return;
    
    uint32_t *pixels = (uint32_t *)fb->address;
    uint64_t line_size = fb->width * sizeof(uint32_t);
    uint64_t font_height_pixels = FONT_HEIGHT;
    
    /* Move all lines up by FONT_HEIGHT pixels */
    for (uint64_t y = font_height_pixels; y < fb->height; y++) {
        memcpy(&pixels[(y - font_height_pixels) * fb->width],
               &pixels[y * fb->width],
               line_size);
    }
    
    /* Clear the last line */
    for (uint64_t y = fb->height - font_height_pixels; y < fb->height; y++) {
        for (uint64_t x = 0; x < fb->width; x++) {
            pixels[y * fb->width + x] = bg_color;
        }
    }
    
    cursor_y -= font_height_pixels;
}

/* Draw character at specific position */
static void fb_draw_char(char c, uint64_t x, uint64_t y, uint32_t fg, uint32_t bg) {
    if (!fb) return;
    
    const uint8_t *glyph = font_8x16[(unsigned char)c];
    
    for (int row = 0; row < FONT_HEIGHT; row++) {
        uint8_t byte = glyph[row];
        for (int col = 0; col < FONT_WIDTH; col++) {
            uint32_t color = (byte & (1 << (7 - col))) ? fg : bg;
            fb_putpixel(x + col, y + row, color);
        }
    }
}

/* Put character at current cursor position */
void fb_putchar(char c) {
    if (!fb) return;
    
    /* Handle special characters */
    if (c == '\n') {
        cursor_x = 0;
        cursor_y += FONT_HEIGHT;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\t') {
        cursor_x = (cursor_x + (FONT_WIDTH * 4)) & ~(FONT_WIDTH * 4 - 1);
    } else if (c == '\b') {
        if (cursor_x >= FONT_WIDTH) {
            cursor_x -= FONT_WIDTH;
        }
    } else {
        /* Print character */
        fb_draw_char(c, cursor_x, cursor_y, fg_color, bg_color);
        cursor_x += FONT_WIDTH;
    }
    
    /* Check for line wrap */
    if (cursor_x >= fb->width) {
        cursor_x = 0;
        cursor_y += FONT_HEIGHT;
    }
    
    /* Check for scroll */
    if (cursor_y + FONT_HEIGHT > fb->height) {
        fb_scroll();
    }
}

/* Put string at current cursor position */
void fb_puts(const char *str) {
    if (!str) return;
    
    while (*str) {
        fb_putchar(*str++);
    }
}

/* Put string with specific color */
void fb_puts_color(const char *str, uint32_t fg, uint32_t bg) {
    if (!str) return;
    
    uint32_t old_fg = fg_color;
    uint32_t old_bg = bg_color;
    
    fg_color = fg;
    bg_color = bg;
    
    fb_puts(str);
    
    fg_color = old_fg;
    bg_color = old_bg;
}

/* Set cursor position */
void fb_set_cursor(uint64_t x, uint64_t y) {
    cursor_x = x * FONT_WIDTH;
    cursor_y = y * FONT_HEIGHT;
}

/* Get framebuffer info */
uint64_t fb_get_width(void) {
    return fb ? fb->width : 0;
}

uint64_t fb_get_height(void) {
    return fb ? fb->height : 0;
}
