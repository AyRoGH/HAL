#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#include "colordata.h"


/**
 * 	BAYER
 */
colordata_bayer_t *colordata_bayer_create(uint32_t height,
                                          uint32_t width,
                                          int      fd)
{
	colordata_bayer_t *bayer;

	if(!height ||
	   !width) {
		return (colordata_bayer_t*)NULL;
	}
	bayer = (colordata_bayer_t*)sbrk(sizeof(colordata_bayer_t));
	if(bayer == (colordata_bayer_t*) - 1) {
		return (colordata_bayer_t*)NULL;
	}
	bayer->height = height;
	bayer->width = width;
	bayer->data = (uint16_t*)mmap(NULL, bayer->height * bayer->width * sizeof(uint16_t), PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, fd, 0);
	return bayer->data == MAP_FAILED ? bayer = (colordata_bayer_t*)sbrk(-sizeof(colordata_bayer_t)), (colordata_bayer_t*)NULL : bayer;
}

int colordata_bayer_destroy(colordata_bayer_t *bayer)
{
	if(!bayer &&
	   (!bayer->data ||
	   !bayer->height ||
	   !bayer->width)) {
		return -1;
	}
	if(munmap((void*)bayer->data, (size_t)(bayer->height * bayer->width) * sizeof(uint16_t))) {
		return -1;
	}
	bayer = (colordata_bayer_t*)sbrk(-sizeof(colordata_bayer_t));
	return 0;
}

int colordata_bayer_assignvalue(colordata_bayer_t **bayer,
                                uint16_t            value,
                                uint32_t            x_pos,
                                uint32_t            y_pos)
{
	return *bayer && (*bayer)->data && x_pos < (*bayer)->height && y_pos < (*bayer)->width ? (*bayer)->data[x_pos * (*bayer)->width + y_pos] = value, 0 : -1;
}

uint16_t colordata_bayer_getvalue(colordata_bayer_t **bayer,
                                  uint32_t            x_pos,
                                  uint32_t            y_pos)
{
	return *bayer && (*bayer)->data && x_pos < (*bayer)->height && y_pos < (*bayer)->width ? (*bayer)->data[x_pos * (*bayer)->width + y_pos] : 0;
}


/**
 * 	RGB
 */
colordata_rgb_t *colordata_rgb_create(uint32_t height,
                                      uint32_t width,
                                      int      fd)
{
	colordata_rgb_t *rgb;

	if(!height ||
	   !width) {
		return (colordata_rgb_t*)NULL;
	}
	rgb = (colordata_rgb_t*)sbrk(sizeof(colordata_rgb_t));
	if(rgb == (void*) - 1) {
		return (colordata_rgb_t*)NULL;
	}
	rgb->height = height;
	rgb->width = width;
	rgb->blue = (uint16_t*)mmap(NULL, (size_t)(rgb->height * rgb->width) * sizeof(uint16_t), PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, fd, 0);
	if(rgb->blue == MAP_FAILED) {
		rgb = (colordata_rgb_t*)sbrk(-sizeof(colordata_rgb_t));
		return (colordata_rgb_t*)NULL;
	}
	rgb->green = (uint16_t*)mmap(NULL, (size_t)(rgb->height * rgb->width) * sizeof(uint16_t), PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, fd, 0);
	if(rgb->blue == MAP_FAILED) {
		munmap((void*)rgb->blue, (size_t)(rgb->height * rgb->width) * sizeof(uint16_t));
		rgb = (colordata_rgb_t*)sbrk(-sizeof(colordata_rgb_t));
		return (colordata_rgb_t*)NULL;
	}
	rgb->red = (uint16_t*)mmap(NULL, (size_t)(rgb->height * rgb->width) * sizeof(uint16_t), PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, fd, 0);
	if(rgb->blue == MAP_FAILED) {
		munmap((void*)rgb->blue, (size_t)(rgb->height * rgb->width) * sizeof(uint16_t));
		munmap((void*)rgb->green, (size_t)(rgb->height * rgb->width) * sizeof(uint16_t));
		rgb = (colordata_rgb_t*)sbrk(-sizeof(colordata_rgb_t));
		return (colordata_rgb_t*)NULL;
	}
	return rgb;
}

int colordata_rgb_destroy(colordata_rgb_t **rgb)
{
	if(!*rgb ||
	   !(*rgb)->blue ||
	   !(*rgb)->green ||
	   !(*rgb)->red ||
	   !(*rgb)->height ||
	   !(*rgb)->width) {
		return -1;
	}
	if(munmap((*rgb)->blue, (size_t)((*rgb)->height * (*rgb)->width) * sizeof(uint16_t)) ||
	   munmap((*rgb)->green, (size_t)((*rgb)->height * (*rgb)->width) * sizeof(uint16_t)) ||
	   munmap((*rgb)->red, (size_t)((*rgb)->height * (*rgb)->width) * sizeof(uint16_t))) {
		return -1;
	}
	*rgb = (colordata_rgb_t*)sbrk(-sizeof(colordata_rgb_t));
	return 0;
}

int colordata_rgb_assignvalue(colordata_rgb_t      **rgb,
                              colordata_rgb_colors   color,
                              uint16_t               value,
                              uint32_t               x_pos,
                              uint32_t               y_pos)
{
	if(color == RGB_BLUE) {
		return *rgb && (*rgb)->blue && x_pos < (*rgb)->height && y_pos < (*rgb)->width ? (*rgb)->red[x_pos * (*rgb)->height + y_pos] = value : -1;
	}
	if(color == RGB_GREEN) {
		return *rgb && (*rgb)->green && x_pos < (*rgb)->height && y_pos < (*rgb)->width ? (*rgb)->red[x_pos * (*rgb)->height + y_pos] = value : -1;
	}
	if(color == RGB_GREEN) {
		return *rgb && (*rgb)->red && x_pos < (*rgb)->height && y_pos < (*rgb)->width ? (*rgb)->red[x_pos * (*rgb)->height + y_pos] = value : -1;
	}
	return -1;
}

uint16_t colordata_rgb_getvalue(colordata_rgb_t      **rgb,
                                colordata_rgb_colors   color,
                                uint32_t               x_pos,
                                uint32_t               y_pos)
{
	if(color == RGB_BLUE) {
		return *rgb && (*rgb)->blue && x_pos < (*rgb)->height && y_pos < (*rgb)->width ? (*rgb)->red[x_pos * (*rgb)->height + y_pos] : 0;
	}
	if(color == RGB_GREEN) {
		return *rgb && (*rgb)->green && x_pos < (*rgb)->height && y_pos < (*rgb)->width ? (*rgb)->red[x_pos * (*rgb)->height + y_pos] : 0;
	}
	if(color == RGB_GREEN) {
		return *rgb && (*rgb)->red && x_pos < (*rgb)->height && y_pos < (*rgb)->width ? (*rgb)->red[x_pos * (*rgb)->height + y_pos] : 0;
	}
	return 0;
}

colordata_functions_t colordata = {
	.bayer = {
		.create = &colordata_bayer_create,
		.destroy = &colordata_bayer_destroy,
		.assignvalue = &colordata_bayer_assignvalue,
		.getvalue = &colordata_bayer_getvalue
	},
	.rgb = {
		.create = &colordata_rgb_create,
		.destroy = &colordata_rgb_destroy,
		.assignvalue = &colordata_rgb_assignvalue,
		.getvalue = &colordata_rgb_getvalue
	}
};

