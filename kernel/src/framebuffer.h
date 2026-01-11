#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdint.h>
#include <stddef.h>
#include "limine.h"

/* Color definitions (RGB) */
#define COLOR_BLACK     0x00000000
#define COLOR_WHITE     0x00FFFFFF
#define COLOR_RED       0x00FF0000
#define COLOR_GREEN     0x0000FF00
#define COLOR_BLUE      0x000000FF
#define COLOR_CYAN      0x0000FFFF
#define COLOR_MAGENTA   0x00FF00FF
#define COLOR_YELLOW    0x00FFFF00
#define COLOR_GRAY      0x00808080

/* Font dimensions */
#define FONT_WIDTH  8
#define FONT_HEIGHT 16

/* Initialize framebuffer */
void fb_init(struct limine_framebuffer *fb);

/* Clear screen with color */
void fb_clear(uint32_t color);

/* Set pixel at coordinates */
void fb_putpixel(uint64_t x, uint64_t y, uint32_t color);

/* Put character at current cursor position */
void fb_putchar(char c);

/* Put string at current cursor position */
void fb_puts(const char *str);

/* Put string with specific color */
void fb_puts_color(const char *str, uint32_t fg, uint32_t bg);

/* Set cursor position */
void fb_set_cursor(uint64_t x, uint64_t y);

/* Get framebuffer info */
uint64_t fb_get_width(void);
uint64_t fb_get_height(void);

#endif /* FRAMEBUFFER_H */
