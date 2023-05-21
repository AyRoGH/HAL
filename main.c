#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include <time.h>

#include "colordata/colordata.h"
#include "ppm/ppm.h"

int main()
{
	clock_t start = clock();
	colordata_bayer_t *bayer = NULL;

	printf("====================\n");
	if(ppm.read2bayer(&bayer, "out/1684423638536.ppm")) {
		printf("ERROR\n");
		return -1;
	}
	if (bayer) {
		printf("%d %d\n", bayer->height, bayer->width);
		for(size_t x = 0; x < bayer->height; x++) {
			for(size_t y = 0; y < bayer->height; y++) {
				printf("%d\t", colordata.bayer.getvalue(&bayer, x, y));
			}
			printf("\n");
		}
	}
	colordata.bayer.destroy(&bayer);
	return 0;
}

