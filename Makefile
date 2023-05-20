CC = gcc

all: out/libcolordata.so out/libppm.so out/main

out/libcolordata.so: colordata/colordata.c outdir
	$(CC) -g -shared -o $@ $< -fPIC

out/libppm.so: ppm/ppm.c outdir
	$(CC) -g -shared -o $@ $< -fPIC -Lout -lcolordata

out/main: main.c
	$(CC) -g -o $@ $< -Lout -lcolordata -lppm

outdir:
	mkdir out

clean:
	rm -rf out/*.so out/main out

