#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>

#include "../colordata/colordata.h"
#include "ppm.h"

/*
 * 	READ
 */

static inline int _open_file_from_path(char *path)
{
	return !path ? -1 : open(path, O_RDONLY);
}

static inline off_t _get_file_size(int fd)
{
	off_t file_size = 0;
	
	if(fd == -1) {
		return -1;
	}
	file_size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	return file_size;
}

static inline uint8_t *_read_file(int   fd,
                                  off_t file_size)
{
	uint8_t *file_data = NULL;
	
	if(fd == -1 || file_size <= 0) {
		return NULL;
	}
	file_data = (uint8_t*)mmap(NULL, (size_t)file_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if(file_data == (uint8_t*)MAP_FAILED) {
		return NULL;
	}
	return file_data;
}

static inline int _magic_number_check(uint8_t** file_data)
{
	return (*file_data)[0] == 'P' && (*file_data)[1] == '6' && (*file_data)[2] == '\n' ? 0 : -1;
}

static inline int _file_parser(uint8_t **file_data,
                               off_t     file_size,
                               uint8_t **height_buf,
                               size_t   *height_buf_size,
                               uint8_t **width_buf,
                               size_t   *width_buf_size,
                               uint8_t **color_depth_buf,
                               size_t   *color_depth_buf_size,
			       uint8_t **data_buf)
{
	off_t file_pos = 3;
	
	if(!*file_data) {
		return -1;
	}
	if((*file_data)[file_pos] == '#') {
		while((*file_data)[file_pos] != '\n') {
			file_pos++;
		}
	}
	(*height_buf) = (*file_data) + file_pos;
	while((*file_data)[file_pos] != ' ') {
		if(file_pos == file_size) {
			(*height_buf) = NULL;
			(*height_buf_size) = 0;
			return -1;
		}
		(*height_buf_size)++;
		file_pos++;
	}
	(*width_buf) = (*file_data) + ++file_pos;
	while((*file_data)[file_pos] != '\n') {
		if(file_pos == file_size) {
			(*height_buf) = NULL;
			(*width_buf) = NULL;
			(*height_buf_size) = 0;
			(*width_buf_size) = 0;
			return -1;
		}
		(*width_buf_size)++;
		file_pos++;
	}
	(*color_depth_buf) = (*file_data) + ++file_pos;
	while((*file_data)[file_pos] != '\n') {
		if(file_pos == file_size) {
			(*height_buf) = NULL;
			(*width_buf) = NULL;
			(*color_depth_buf) = NULL;
			(*height_buf_size) = 0;
			(*width_buf_size) = 0;
			(*color_depth_buf_size) = 0;
		}
		(*color_depth_buf_size)++;
		file_pos++;
	}
	(*data_buf) = (*file_data) + ++file_pos;
	return height_buf_size || width_buf_size || color_depth_buf_size ? 0 : -1;
}

static inline uint32_t _str_2_uint32(uint8_t **str,
                                     size_t    str_size)
{
	uint64_t pow10 = 1, result = 0;
	
	if(!*str || str_size > 9) {
		return 0;
	}
	for(size_t i = 1; i <= str_size; i++) {
		if((*str)[str_size - i] < '0' ||
		   (*str)[str_size - i] > '9') {
			return 0;
		}
		result += (uint64_t)((*str)[str_size - i] - 48) * pow10;
		pow10 *= 10;
		if(pow10 == 1000000000 ||
		   result > (uint64_t)UINT32_MAX) {
			return 0;
		}
	}
	return (uint32_t)result;
}

static inline uint16_t _dbluint8_2_uint16(uint8_t byte1,
                                          uint8_t byte2)
{
	return (uint16_t)(byte1 << 8) + (uint16_t)byte2;
}

static inline int _data_2_bayer(uint8_t           **data_buf,
                                colordata_bayer_t **bayer)
{
	uint64_t data_index = 0;
	
	if(!*data_buf ||
	   !*bayer) {
		return -1;
	}
	for(uint32_t x = 0; x < (*bayer)->height; x++) {
		for(uint32_t y = 0; y < (*bayer)->width; y++) {
			if(data_index < (uint64_t)((*bayer)->height * (*bayer)->width) &&
			   colordata.bayer.assignvalue(bayer, _dbluint8_2_uint16((*data_buf)[data_index], (*data_buf)[data_index + 1]), x, y)) {
				return -1;
			}
			data_index+=2;
		}
	}
	return 0;
}

int ppm_bayer_read(colordata_bayer_t **bayer,
                   char               *path)
{
	int fd = 0;
	off_t file_size = 0;
	size_t height_buf_size = 0, width_buf_size = 0, color_depth_buf_size = 0;
	uint8_t *file_data = NULL, *height_buf = NULL, *width_buf = NULL, *color_depth_buf = NULL, *data_buf = NULL;
	uint32_t height = 0, width = 0, color_depth = 0;
	
	fd = _open_file_from_path(path);
	if(fd == -1) {
		close(fd);
		return -1;
	}
	file_size = _get_file_size(fd);
	if(file_size == -1) {
		close(fd);
		return -1;
	}
	file_data = _read_file(fd, file_size);
	if(!file_data) {
		close(fd);
		return -1;
	}
	if(_magic_number_check(&file_data)) {
		munmap(file_data, (size_t)file_size);
		close(fd);
		return -1;
	}
	if(_file_parser(&file_data, file_size, &height_buf, &height_buf_size, &width_buf, &width_buf_size, &color_depth_buf, &color_depth_buf_size, &data_buf) == -1){
		munmap(file_data, (size_t)file_size);
		close(fd);
		return -1;
	}
	height = _str_2_uint32(&height_buf, height_buf_size);
	width = _str_2_uint32(&width_buf, width_buf_size);
	color_depth = _str_2_uint32(&color_depth_buf, color_depth_buf_size);
	if(!height ||
	   !width ||
	   !color_depth) {
		munmap(file_data, (size_t)file_size);
		close(fd);
		return -1;
	}
	(*bayer) = colordata.bayer.create(height, width, -1);
	if(!bayer) {
		munmap(file_data, (size_t)file_size);
		close(fd);
		return -1;
	}
	if(_data_2_bayer(&data_buf, bayer)) {
		munmap(file_data, (size_t)file_size);
		close(fd);
		colordata.bayer.destroy(bayer);
		return -1;
	}
	munmap(file_data, (size_t)file_size);
	close(fd);
	return 0;
}

/*
 * 	WRITE
 */

static inline size_t _get_uint32_str_size(uint32_t value)
{
	size_t str_size = 0;
	uint64_t pow10 = 1;
	
	while((uint64_t)value/pow10) {
		pow10 *= 10;
		str_size++;
	}
	return str_size;
}

static inline int _put_uint32_2_buf(uint8_t  **buf,
                                    uint32_t   value,
                                    size_t     start_pos,
                                    size_t     value_buf_size)
{
	uint64_t pow10 = 1;
	
	if(!*buf ||
	   !value_buf_size ||
	   value_buf_size > 20) {
		return -1;
	}
	for(size_t i = 0; i < value_buf_size - 1; i++) {
		pow10 *= 10;
	}
	for(size_t i = 0; i < value_buf_size; i++) {
		(*buf)[i + start_pos] = (uint8_t)((uint64_t)value/pow10) + 48;
		value -= (value/pow10) * pow10;
		pow10 /= 10;
	}
	return 0;
}

static inline int _data_2_buf(colordata_bayer_t **bayer,
                              uint8_t           **buf,
                              size_t              index)
{
	uint8_t byte1 = 0, byte2 = 0;
	uint16_t mask1 = (uint16_t)UINT8_MAX << 8, mask2 = (uint16_t)UINT8_MAX;
	
	if(!*bayer || !*buf) {
		return -1;
	}
	for(size_t x = 0; x < (*bayer)->height; x++) {
		for(size_t y = 0; y < (*bayer)->width; y++) {
			byte1 = (uint8_t)((colordata.bayer.getvalue(bayer, x, y) & mask1) >> 8);
			byte2 = (uint8_t)((colordata.bayer.getvalue(bayer, x, y) & mask2) >> 8);
			(*buf)[index] = byte1;
			(*buf)[index + 1] = byte2;
			index+=2;
		}
	}
	return 0;
}

int ppm_bayer_write(colordata_bayer_t **bayer,
                    char               *path)
{
	int fd = 0;
	size_t write_buf_size = 0, height_buf_size = 0, width_buf_size = 0;
	uint8_t *write_buf = NULL;
	uint64_t write_index = 0;
	
	if(!*bayer ||
	   !path ||
	   !(*bayer)->height ||
	   !(*bayer)->width) {
		return -1;
	}
	fd = open(path, O_CREAT | O_TRUNC | O_RDWR);
	if(fd == -1) {
		return -1;
	}
	height_buf_size = _get_uint32_str_size((*bayer)->height);
	width_buf_size = _get_uint32_str_size((*bayer)->width);
	write_buf_size = 12 + height_buf_size + width_buf_size + ((size_t)(*bayer)->height * (size_t)(*bayer)->width) * 6;
	if(lseek(fd, write_buf_size - 1, SEEK_SET) == -1) {
		close(fd);
		return -1;
	}
	if(write(fd, "", 1) == -1) {
		close(fd);
		return -1;
	}
	write_buf = (uint8_t*)mmap(NULL, write_buf_size, PROT_WRITE, MAP_SHARED, fd, 0);
	if(write_buf == MAP_FAILED) {
		perror("mmap");
		return -1;
	}
	
	write_buf[0] = 'P';
	write_buf[1] = '6';
	write_buf[2] = '\n';
	if(_put_uint32_2_buf(&write_buf, (*bayer)->height, 3, height_buf_size)) {
		munmap(write_buf, write_buf_size);
		return -1;
	}
	write_buf[3 + height_buf_size] = ' ';
	if(_put_uint32_2_buf(&write_buf, (*bayer)->width, 4 + height_buf_size, width_buf_size)) {
		munmap(write_buf, write_buf_size);
		return -1;
	}
	write_buf[4 + height_buf_size + width_buf_size] = '\n';
	if(_put_uint32_2_buf(&write_buf, UINT16_MAX, 5 + height_buf_size + width_buf_size, 5)) {
		munmap(write_buf, write_buf_size);
		return -1;
	}
	write_buf[10 + height_buf_size + width_buf_size] = '\n';
	write_index = height_buf_size + width_buf_size + 11;
	if(_data_2_buf(bayer, &write_buf, write_index)) {
		munmap(write_buf, write_buf_size);
		return -1;
	}
	munmap(write_buf, write_buf_size);
	close(fd);
	return 0;
}

ppm_functions_t ppm = {
	.bayer_read = &ppm_bayer_read,
	.bayer_write = &ppm_bayer_write
};
