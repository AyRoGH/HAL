#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>

#include "../colordata/colordata.h"
#include "ppm.h"

static int bayer_read_hdlr(colordata_bayer_t **bayer,
                           uint8_t            *read_data,
                           uint32_t            height,
                           uint32_t            width)
{
	*bayer = colordata.bayer.create(height, width, -1);
	if(!*bayer) {
		return -1;
	}
	printf(" => %d %d\n", (*bayer)->height, (*bayer)->width);
	for(size_t x = 0; x < (*bayer)->height; x++) {
		for(size_t y = 0; y < (*bayer)->height; y++) {
			colordata.bayer.assignvalue(bayer,
	 					    (uint16_t)((read_data[x * (*bayer)->width + y]) << 16) + (uint16_t)(read_data[x * (*bayer)->width + y + 1]),
	 					    x / 2,
	 					    y / 2);
		}
	}
	return 0;
}

static inline uint32_t ascii_str2uint32(uint8_t *string)
{
	size_t string_size = 0;
	uint32_t result = 0;
	uint64_t pow_10 = 1;
	
	if(!string) {
		return 0;
	}
	while(string[string_size]) {
		string_size++;
	}
	for(size_t i = 1; i <= string_size; i++) {
		if(string[string_size - i] < 48 ||
		   string[string_size - i] > 57) {
			return 0;
		}
		result += ((uint32_t)string[string_size - i] - 48) * pow_10;
		pow_10 *= 10;
	}
	return result;
}

int ppm_read2bayer(colordata_bayer_t **bayer,
                   char              *path)
{
	int fd = 0, header_size = 3, i = header_size;
	off_t file_size;
	size_t height_buf_size = 0, width_buf_size = 0, max_color_lvl_buf_size = 0;
	uint8_t *read_buf, *height_buf, *width_buf, *max_color_lvl_buf;
	uint32_t image_height = 0, image_width = 0;
	
	if(!path) {
		return -1;
	}
	fd = open(path, O_RDWR);
	if(fd == -1) {
		return -1;
	}
	file_size = lseek(fd, 0, SEEK_END);
	if((size_t)file_size <= header_size) {
		close(fd);
		return -1;
	}
	lseek(fd, 0, SEEK_SET);
	read_buf = (uint8_t*)sbrk((size_t)file_size * sizeof(uint8_t));
	if(read_buf == (void*) - 1) {
		close(fd);
		return -1;
	}
	if(read(fd, (void*)read_buf, file_size - 1) == -1) {
		read_buf = (uint8_t*)sbrk(-(size_t)file_size * sizeof(uint8_t));
		close(fd);
		return -1;
	}
	close(fd);
	if(read_buf[0] != 'P' ||
	   read_buf[1] != '6' ||
	   read_buf[2] != 0x0a) {
		read_buf = (uint8_t*)sbrk(-(size_t)file_size * sizeof(uint8_t));
		close(fd);
		return -1;
	}
	while(read_buf[header_size] != 0x20) {
		height_buf_size++;
		header_size++;
	}
	header_size++;
	while(read_buf[header_size] != 0x0a) {
		width_buf_size++;
		header_size++;
	}
	header_size++;
	while(read_buf[header_size] != 0x0a) {
		max_color_lvl_buf_size++;
		header_size++;
	}
	height_buf = (uint8_t*)sbrk(height_buf_size * sizeof(uint8_t));
	if(height_buf == (void*) - 1) {
		read_buf = (uint8_t*)sbrk(-file_size * sizeof(uint8_t));
		return -1;
	}
	width_buf = (uint8_t*)sbrk(width_buf_size * sizeof(uint8_t));
	if(width_buf == (void*) - 1) {
		read_buf = (uint8_t*)sbrk(-file_size * sizeof(uint8_t));
		height_buf = (uint8_t*)sbrk(-height_buf_size * sizeof(uint8_t));
		return -1;
	}
	max_color_lvl_buf = (uint8_t*)sbrk(width_buf_size * sizeof(uint8_t));
	if(max_color_lvl_buf == (void*) - 1) {
		read_buf = (uint8_t*)sbrk(-file_size * sizeof(uint8_t));
		height_buf = (uint8_t*)sbrk(-height_buf_size * sizeof(uint8_t));
		width_buf = (uint8_t*)sbrk(-width_buf_size * sizeof(uint8_t));
		return -1;
	}
	while(read_buf[i] != 0x20) {
		height_buf[i - 3] = read_buf[i];
		i++;
	}
	image_height = ascii_str2uint32(height_buf);
	height_buf = (uint8_t*)sbrk(-height_buf_size * sizeof(uint8_t));
	i++;
	while(read_buf[i] != 0x0a) {
		width_buf[i - height_buf_size - 4] = read_buf[i];
		i++;
	}
	image_width = ascii_str2uint32(width_buf);
	width_buf = (uint8_t*)sbrk(-width_buf_size * sizeof(uint8_t));
	i++;
	while(read_buf[i] != 0x0a) {
		max_color_lvl_buf[i - height_buf_size - width_buf_size - 5] = read_buf[i];
		i++;
	}
	header_size++;
	if(ascii_str2uint32(max_color_lvl_buf) != (uint32_t)UINT16_MAX) {
		read_buf = (uint8_t*)sbrk(-file_size * sizeof(uint8_t));
		return -1;
	}
	max_color_lvl_buf = (uint8_t*)sbrk(-max_color_lvl_buf_size * sizeof(uint8_t));
	if(header_size + (size_t)(image_height * image_width) * 6 != file_size) {
		read_buf = (uint8_t*)sbrk(-(size_t)file_size * sizeof(uint8_t));
		return -1;
	}
	/* *bayer = colordata.bayer.create(image_height, image_width, -1); */
	/* if(!*bayer) { */
	/* 	read_buf = (uint8_t*)sbrk(-file_size * sizeof(uint8_t)); */
	/* 	return -1; */
	/* } */
	/* for(uint32_t x = 0; x < (*bayer)->height; x += 2) { */
	/* 	for(uint32_t y = 0; y < (*bayer)->width * 2; y += 2) { */
	/* 		colordata.bayer.assignvalue(bayer, */
	/* 					    (uint16_t)((read_buf[x * (*bayer)->width + y + header_size]) << 16) + (uint16_t)(read_buf[x * (*bayer)->width + y + header_size + 1]), */
	/* 					    x / 2, */
	/* 					    y / 2); */
	/* 	} */
	/* } */
	/* printf(" => %d %d\n", (*bayer)->height, (*bayer)->width); */
	/* read_buf = (uint8_t*)sbrk(-(size_t)file_size * sizeof(uint8_t)); */
	
	if(bayer_read_hdlr(bayer, (uint8_t*)(read_buf + height_buf_size), image_height, image_width)) {
		read_buf = (uint8_t*)sbrk(-(size_t)file_size * sizeof(uint8_t));
		return -1;
	}
	read_buf = (uint8_t*)sbrk(-(size_t)file_size * sizeof(uint8_t));
	return 0;
}

void test(colordata_bayer_t **bayer,
          uint32_t            height,
          uint32_t            width) {
	*bayer = colordata.bayer.create(height, width, -1);
	printf(" => %d %d\n", (*bayer)->height, (*bayer)->width);
	for(size_t x = 0; x < (*bayer)->height; x++) {
		for(size_t y = 0; y < (*bayer)->height; y++) {
			colordata.bayer.assignvalue(bayer, x * y, x, y);
		}
	}
}

/* int ppm_writebayer(colordata_bayer_t *bayer, */
/*                    uint8_t           *path) */
/* { */
	
/* } */

ppm_functions_t ppm = {
	.read2bayer = &ppm_read2bayer,
	.test = &test
};
