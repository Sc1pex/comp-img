#!/bin/sh

make clean
make

./generate_image 3840 2160 i1.bin
./generate_image 3840 2160 i2.bin

hyperfine -N -L threads 1,2,4,8 "./compare_images3 i1.bin i2.bin 3840 2160 {threads}"
