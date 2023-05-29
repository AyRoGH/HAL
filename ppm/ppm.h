#include <stdint.h>
#include "../colordata/colordata.h"

#ifndef __PPM_H__
#define __PPM_H__

typedef struct {
        int (*bayer_read)(colordata_bayer_t **bayer, char *path);
        int (*bayer_write)(colordata_bayer_t **bayer, char *path);
} ppm_functions_t;

extern ppm_functions_t ppm;

#endif /* __PPM_H__ */
