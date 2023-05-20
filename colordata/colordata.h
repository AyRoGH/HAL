#include <stdint.h>

#ifndef __COLORDATA_H__
#define __COLORDATA_H__

typedef struct {
        uint16_t *data;
        uint32_t height;
        uint32_t width;
        uint64_t original_color_depth;
} colordata_bayer_t;

typedef struct {
        uint16_t *blue;
        uint16_t *green;
        uint16_t *red;
        uint32_t height;
        uint32_t width;
        uint64_t original_color_depth;
} colordata_rgb_t;

typedef enum {
        RGB_BLUE = 0x0001,
        RGB_GREEN = 0x0010,
        RGB_RED = 0x0100
} colordata_rgb_colors;

typedef struct {
        struct {
                colordata_bayer_t *(*create)(uint32_t height, uint32_t width, int fd);
                int (*destroy)(colordata_bayer_t** bayer_image);
                int (*assignvalue)(colordata_bayer_t** bayer_image, uint16_t value, uint32_t x, uint32_t y);
                uint16_t (*getvalue)(colordata_bayer_t** bayer_image, uint32_t x, uint32_t y);
        } bayer;
        struct {
                colordata_rgb_t *(*create)(uint32_t height, uint32_t width, int fd);
                int (*destroy)(colordata_rgb_t** rgb_image);
                int (*assignvalue)(colordata_rgb_t** rgb_image, colordata_rgb_colors color, uint16_t value, uint32_t x, uint32_t y);
                uint16_t (*getvalue)(colordata_rgb_t **rgb_image, colordata_rgb_colors color, uint32_t x, uint32_t y);
        } rgb;
} colordata_functions_t;

extern colordata_functions_t colordata;

#endif /* __COLORDATA_H__ */
