#ifndef COLOR_H
#define COLOR_H

#include <stdint.h>

typedef uint32_t color_t;

static inline color_t
color_pack(uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)a << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

static inline uint8_t
color_get_alpha(color_t color) {
    return (color >> 24) & 0xFF;
}

static inline uint8_t
color_get_red(color_t color) {
    return (color >> 16) & 0xFF;
}

static inline uint8_t
color_get_green(color_t color) {
    return (color >> 8) & 0xFF;
}

static inline uint8_t
color_get_blue(color_t color) {
    return color & 0xFF;
}

#endif
