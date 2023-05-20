#include <stdint.h>
#include "../colordata/colordata.h"

#ifndef __PPM_H__
#define __PPM_H__

typedef struct {
        int (*read2bayer)(colordata_bayer_t **bayer, char *path);
        void (*test)(colordata_bayer_t **bayer);
} ppm_functions_t;

extern ppm_functions_t ppm;

#endif /* __PPM_H__ */
