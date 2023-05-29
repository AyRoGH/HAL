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

	if(ppm.bayer_read(&bayer, "out/1684423638536.ppm")) {
		printf("ERROR\n");
		return -1;
	}
	if(ppm.bayer_write(&bayer, "out/test.ppm")) {
		printf("ERROR\n");
		return -1;
	}
	colordata.bayer.destroy(&bayer);
	return 0;
}

