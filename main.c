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

	if (ppm.read2bayer(&bayer, "out/1684423638536.ppm") != 0) {
		printf("ERROR\n");
		return 1;
	}
	if (bayer) {
		printf("%d %d\n", bayer->height, bayer->width);
		printf("%.2x\n", colordata.bayer.getvalue(&bayer, 1000, 1000));
	}
	colordata.bayer.destroy(&bayer);
	return 0;
}

